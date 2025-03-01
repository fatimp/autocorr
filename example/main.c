#include <stdlib.h>
#include <stdio.h>
#include <autocorr.h>

int main (int argc, const char *argv[]) {
    if (argc < 3) {
        fprintf (stderr,
                 "Usage: autocorr-example "
                 "<sequence of 1's and 0's, minimal length = 2>\n" );
        return 1;
    }

    struct ac_buffers *buffers = ac_alloc (argc - 1);
    uint64_t *src = ac_get_src (buffers);
    uint64_t *dst = ac_get_dst (buffers);

    if (buffers == NULL) {
        fprintf (stderr, "Cannot allocate buffers\n" );
        return 1;
    }

    for (int i = 0; i < argc - 1; i++) {
        src[i] = strtol (argv[i + 1], NULL, 10);
        if (src[i] > 1) {
            fprintf (stderr, "Sequence must contain only 0's or 1's\n");
            ac_free (buffers);
            return 1;
        }
    }

    ac_autocorr (buffers);
    for (int i = 0; i < ac_autocorr_length (argc - 1); i++) {
        printf ("%lu ", dst[i]);
    }
    printf ("\n");

    ac_free (buffers);
    return 0;
}
