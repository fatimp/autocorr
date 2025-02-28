#include <x86intrin.h>
#include <string.h>
#include "autocorr.h"
#include "generated.h"

static uint32_t nsteps (uint32_t x) {
    return _bit_scan_reverse(x - 1) + 1;
}

size_t ac_autocorr_length (size_t length) {
    return 1 << nsteps(2*length - 1);
}

static acfn get_autocorr_fn (size_t length) {
    acfn res = NULL;

    for (int i = 0; i < PARAM_LENGTH; i++) {
        if (parameter_array[i].length == length) {
            res = parameter_array[i].fn;
            break;
        }
    }
    
    return res;
}

struct ac_buffers {
    uint64_t *src;
    uint64_t *dst;
    acfn      fn;
};

struct ac_buffers* ac_alloc (size_t length) {
    struct ac_buffers* buffers;

    length = ac_autocorr_length (length);
    acfn fn = get_autocorr_fn (length);
    if (fn == NULL) {
        return NULL;
    }

    buffers = malloc(sizeof (struct ac_buffers));
    buffers->src = malloc(length * sizeof (uint64_t));
    buffers->dst = malloc(length * sizeof (uint64_t));
    buffers->fn  = fn;
    memset (buffers->src, 0, sizeof (uint64_t) * length);

    return buffers;
}

void ac_free (struct ac_buffers *buffers) {
    free (buffers->src);
    free (buffers->dst);
    free (buffers);
}

void ac_autocorr (struct ac_buffers *buffers) {
    buffers->fn (buffers->src, buffers->dst);
}

uint64_t *ac_get_src (const struct ac_buffers *buffers) {
    return buffers->src;
}

uint64_t *ac_get_dst (const struct ac_buffers *buffers) {
    return buffers->dst;
}
