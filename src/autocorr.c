#include <x86intrin.h>
#include <string.h>
#include "autocorr.h"
#include "consts.h"

static uint32_t nsteps (uint32_t x) {
    return _bit_scan_reverse(x - 1) + 1;
}

/*
 * size is a power of two.
 * omega is a size-th primitive root of unity in F_p
 */
static void _fft  (uint64_t *array, uint64_t *omegas, uint64_t steps) {
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
                uint64_t m_odd = (odd * _m) % P;
                array[even_idx] = (P + even + m_odd) % P;
                array[odd_idx]  = (P + even - m_odd) % P;
                _m = (_m * m) % P;
            }
        }
    }
}

static uint32_t reverse_bits (uint32_t n, uint64_t steps) {
    uint32_t result = 0;

    for (uint32_t i = 0; i < steps; i++) {
        uint32_t bit = ((1<<i) & n) >> i;
        result |= bit << (steps - i - 1);
    }

    return result;
}

// size is a power of two.
static void reorder_input (const uint64_t *src, uint64_t *dst, size_t length) {
    uint32_t steps = nsteps (length);

    for (size_t i = 0; i < length; i++) {
        uint32_t j = reverse_bits(i, steps);
        if (i <= j) {
            uint64_t n = src[i];
            uint64_t m = src[j];
            dst[j] = n;
            dst[i] = m;
        }
    }
}

static void fft (const uint64_t *src, uint64_t *dst, size_t length) {
    reorder_input (src, dst, length);
    _fft (dst, _omegas, nsteps (length));
}

static void renormalize (uint64_t *array, size_t length) {
    uint32_t steps = nsteps (length);
    uint64_t m = _inv_lengths[steps - 1];

    for (size_t i = 0; i < length; i++) {
        array[i] = (array[i] * m) % P;
    }
}

static void ifft (const uint64_t *src, uint64_t *dst, size_t length) {
    reorder_input (src, dst, length);
    _fft (dst, _inv_omegas, nsteps (length));
    renormalize (dst, length);
}

void autocorr (const uint64_t *src, uint64_t *dst, size_t length) {
    fft (src, dst, length);

    size_t hl = length / 2;
    uint32_t steps = nsteps (length);
    uint64_t inv = _inv_omegas[steps - 1];
    dst[0]  = (dst[0]  * dst[0]) % P;
    dst[hl] = ((dst[hl] * dst[hl] % P) * (P - 1)) % P;
    uint64_t omega = _omegas[steps - 1];
    uint64_t m1 = inv;
    uint64_t m2 = omega;
    for (size_t i = 1; i < hl; i++) {
        size_t j = length - i;
        uint64_t low  = dst[i];
        uint64_t high = dst[j];

        dst[i] = (((low * high) % P) * m1) % P;
        dst[j] = (((low * high) % P) * m2) % P;

        m1 = (m1 * inv) % P;
        m2 = (m2 * omega) % P;
    }

    ifft (dst, dst, length);
}

// Public interface

struct ac_buffers {
    uint64_t *src;
    uint64_t *dst;
    size_t length;
};

size_t ac_autocorr_length (size_t length) {
    return 1 << nsteps (2*length - 1);
}

struct ac_buffers* ac_alloc (size_t length) {
    struct ac_buffers* buffers;

    length = ac_autocorr_length (length);
    uint32_t steps = nsteps (length);

    if (steps > MAX_STEPS) {
        return NULL;
    }

    buffers = malloc(sizeof (struct ac_buffers));
    buffers->length = length;
    buffers->src = malloc(length * sizeof (uint64_t));
    buffers->dst = malloc(length * sizeof (uint64_t));
    memset (buffers->src, 0, sizeof (uint64_t) * length);

    return buffers;
}

void ac_free (struct ac_buffers *buffers) {
    free (buffers->src);
    free (buffers->dst);
    free (buffers);
}

void ac_autocorr (struct ac_buffers *buffers) {
    autocorr (buffers->src, buffers->dst, buffers->length);
}

uint64_t *ac_get_src (const struct ac_buffers *buffers) {
    return buffers->src;
}

uint64_t *ac_get_dst (const struct ac_buffers *buffers) {
    return buffers->dst;
}
