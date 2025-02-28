#include <x86intrin.h>
#include <string.h>
#include "autocorr.h"
#include "internal.h"

struct gcdex_res {
    int64_t gcd;
    int64_t a;
    int64_t b;
};

static struct gcdex_res
gcdex_ (int64_t u, int64_t v,
        int64_t s0, int64_t s1,
        int64_t d0, int64_t d1) {
    div_t tmp = div (u, v);
    int64_t q = tmp.quot;
    int64_t r = tmp.rem;
    int64_t s = s0 - q * s1;
    int64_t d = d0 - q * d1;
    if (r == 0) {
        struct gcdex_res res;
        res.gcd = labs(v);
        res.a = (v > 0) ? s1: -s1;
        res.b = (v > 0) ? d1: -d1;
        return res;
    } else {
        return gcdex_ (v, r, s1, s, d1, d);
    }
}

static struct gcdex_res gcdex (int64_t u, int64_t v) {
    struct gcdex_res res;

    if (u == 0) {
        res.gcd = labs (v);
        res.a = 0;
        res.b = (v > 0) ? 1 : 0;
    } else if (v == 0) {
        res.gcd = labs (u);
        res.a = (u > 0) ? 1 : 0;
        res.b = 0;
    } else if (labs (u) > labs (v)) {
        res = gcdex_ (u, v, 1, 0, 0, 1);
    } else {
        res = gcdex_ (v, u, 1, 0, 0, 1);
        int64_t tmp = res.a;
        res.a = res.b;
        res.b = tmp;
    }

    return res;
}

static uint64_t invert_integer (uint64_t n, uint64_t q) {
    if (n == 0) return 0;

    struct gcdex_res res = gcdex (n, q);
    if (res.gcd != 1) return 0;

    return (res.a + q) % q;
}

static uint32_t nsteps (uint32_t x) {
    return _bit_scan_reverse(x - 1) + 1;
}

static void calc_omegas (uint64_t *omegas, size_t length, uint64_t omega, uint64_t p) {
    omegas[length - 1] = omega;
    for (size_t i = 1; i < length; i++) {
        uint64_t prev = omegas[length-i];
        omegas[length-1-i] = (prev*prev) % p;
    }
}

/*
 * size is a power of two.
 * omega is a size-th primitive root of unity in F_p
 */
static void _fft  (uint64_t *array,
                   size_t   length,
                   uint64_t omega,
                   uint64_t p) {
    uint32_t steps  = nsteps (length);
    uint64_t *omegas = alloca(sizeof(uint64_t) * steps);
    calc_omegas (omegas, steps, omega, p);

    for (size_t s = 0; s < steps; s++) {
        size_t pair_offset = 1 << s;
        size_t ngroups = 1 << (steps-s-1);
        size_t group_size = pair_offset;
        uint64_t m = omegas[s];
        for (size_t i = 0; i < ngroups; i++) {
            size_t group_offset = group_size * 2 * i;
            uint64_t _m = 1;
            for (size_t j = 0; j < group_size; j++) {
                size_t even_idx = group_offset + j;
                size_t odd_idx  = even_idx + pair_offset;
                uint64_t even = array[even_idx];
                uint64_t odd  = array[odd_idx];
                uint64_t m_odd = (odd * _m) % p;
                array[even_idx] = (p + even + m_odd) % p;
                array[odd_idx]  = (p + even - m_odd) % p;
                _m = (_m * m) % p;
            }
        }
    }
}

static uint32_t reverse_bits (uint32_t length, uint32_t n) {
    length = nsteps(length);
    uint32_t result = 0;

    for (uint32_t i = 0; i < length; i++) {
        uint32_t bit = ((1<<i) & n) >> i;
        result |= bit << (length - i - 1);
    }

    return result;
}

// size is a power of two.
static void reorder_input (const uint64_t *src, uint64_t *dst, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uint32_t j = reverse_bits(length, i);
        if (i <= j) {
            uint64_t n = src[i];
            uint64_t m = src[j];
            dst[j] = n;
            dst[i] = m;
        }
    }
}

static int fft (const uint64_t *src, uint64_t *dst, size_t length,
                uint64_t omega, uint64_t p) {
    if (((length - 1) & length) != 0) {
        return 0;
    }

    reorder_input (src, dst, length);
    _fft (dst, length, omega, p);

    return 1;
}

static void renormalize (uint64_t *array, size_t length, uint64_t p) {
    uint64_t inv = invert_integer (length, p);

    for (size_t i = 0; i < length; i++) {
        array[i] = (array[i] * inv) % p;
    }
}

static int ifft (const uint64_t *src, uint64_t *dst, size_t length,
                 uint64_t omega, uint64_t p) {
    if (((length - 1) & length) != 0) {
        return 0;
    }

    uint64_t inv = invert_integer (omega, p);
    reorder_input (src, dst, length);
    _fft (dst, length, inv, p);
    renormalize (dst, length, p);

    return 1;
}

size_t ac_autocorr_length (size_t length) {
    return 1 << nsteps(2*length - 1);
}

static const struct params* get_autocorr_parameters (size_t length) {
    struct params* res;

    for (int i = 0; i < PARAM_LENGTH; i++) {
        res = &parameter_array[i];
        if (res->length == length) {
            return res;
        }
    }
    
    return NULL;
}

struct ac_buffers {
    uint64_t *src;
    uint64_t *dst;
    const struct params *param;
};

struct ac_buffers* ac_alloc (size_t length) {
    struct ac_buffers* buffers;

    length = ac_autocorr_length (length);
    const struct params *param =
        get_autocorr_parameters (length);
    if (param == NULL) {
        return NULL;
    }

    buffers = malloc(sizeof (struct ac_buffers));
    buffers->param = param;
    buffers->src  = malloc(length * sizeof (uint64_t));
    buffers->dst  = malloc(length * sizeof (uint64_t));
    memset (buffers->src, 0, sizeof (uint64_t) * length);

    return buffers;
}

void ac_free (struct ac_buffers *buffers) {
    free (buffers->src);
    free (buffers->dst);
    free (buffers);
}

int ac_autocorr (size_t length, struct ac_buffers *buffers) {
    size_t aclength = ac_autocorr_length (length);
    const struct params *param = buffers->param;

    if (aclength != param->length) {
        return 0;
    }

    if (fft (buffers->src, buffers->dst, param->length, param->omega, param->p) == 0) {
        return 0;
    }

    size_t hl = aclength / 2;
    uint64_t inv = invert_integer (param->omega, param->p);
    buffers->dst[0]  = (buffers->dst[0]  * buffers->dst[0]) % param->p;
    buffers->dst[hl] = (buffers->dst[hl] * buffers->dst[hl] * (param->p - 1)) % param->p;
    uint64_t m1 = inv;
    uint64_t m2 = param->omega;
    for (size_t i = 1; i < hl; i++) {
        size_t j = aclength - i;
        uint64_t low  = buffers->dst[i];
        uint64_t high = buffers->dst[j];

        buffers->dst[i] = (low * high * m1) % param->p;
        buffers->dst[j] = (low * high * m2) % param->p;

        m1 = (m1 * inv) % param->p;
        m2 = (m2 * param->omega) % param->p;
    }

    return ifft (buffers->dst, buffers->dst, param->length, param->omega, param->p);
}

uint64_t *ac_get_src (const struct ac_buffers *buffers) {
    return buffers->src;
}

uint64_t *ac_get_dst (const struct ac_buffers *buffers) {
    return buffers->dst;
}
