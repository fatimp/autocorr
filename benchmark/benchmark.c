#include <autocorr.h>
#include "ubench.h"

#define BENCH(len) UBENCH_EX(fft, length_##len) {       \
        struct ac_buffers *buffers = ac_alloc (len);    \
        UBENCH_DO_BENCHMARK() {                         \
            ac_autocorr(buffers);                       \
        }                                               \
    }

BENCH (256)
BENCH (2046)
BENCH (32768)
BENCH (65536)
BENCH (131072)
BENCH (262144)
BENCH (524288)
BENCH (1048576)

UBENCH_MAIN();
