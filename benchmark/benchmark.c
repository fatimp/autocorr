#include <sys/timespec.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <autocorr.h>

static double gettime ()
{
    struct timespec tp;
    clock_gettime (CLOCK_MONOTONIC, &tp);
    return (double)tp.tv_sec + (0.000000001 * (double)tp.tv_nsec);
}

int main () {
#define N 30000
#define M 5000
    struct ac_buffers *buffers = ac_alloc (N);
    assert (buffers != NULL);

    uint64_t *src = ac_get_src (buffers);
    for (int i = 0; i < N; i++) {
        src[i] = random() & 1;
    }

    double time = gettime ();
    for (int i = 0; i < M; i++) {
        ac_autocorr (buffers);
    }
    time = gettime() - time;

    ac_free (buffers);
    printf ("Length %i, time = %f\n", N, time / M);

    return 0;
}
