#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <autocorr.h>
#include <generated.h>

static int primep (uint64_t p) {
    uint64_t n = 2;

    while (1) {
        uint64_t sqr = n * n;
        if (sqr > p) {
            return 1;
        }

        if (p % n == 0) {
            return 0;
        }

        n++;
    }
}

static uint64_t expt (uint64_t a, uint64_t n, uint64_t acc, uint64_t q) {
    if (n == 0) {
        return acc;
    } else if (n % 2 == 0) {
        return expt ((a * a) % q, n / 2, acc, q);
    } else {
        return expt (a, n - 1, (a * acc) % q, q);
    }
}

static uint64_t autocorr_in_point (uint64_t *array, size_t length, size_t offset) {
    uint64_t res = 0;

    for (size_t idx = 0; idx < length - offset; idx++) {
        if (array[idx] + array[idx + offset] == 2) {
            res++;
        }
    }

    return res;
}

static void test_params () {
    for (int i = 0; i < PARAM_LENGTH; i++) {
        struct params *param = &parameter_array[i];
        CU_ASSERT ((param->length & (param->length - 1)) == 0);
        CU_ASSERT ((param->omega * param->inv_omega) % param->p == 1);
        CU_ASSERT ((param->length * param->inv_length) % param->p == 1);
        CU_ASSERT (expt(param->omega, param->length, 1, param->p) == 1);
        CU_ASSERT (primep (param->p));
    }
}

#define N 5000
#define M 1000

static void test_autocorr () {
    for (int i = 0; i < M; i++) {
        size_t n = 3 + arc4random_uniform (N);
        struct ac_buffers *buffers = ac_alloc (n);
        uint64_t *src = ac_get_src (buffers);
        uint64_t *dst = ac_get_dst (buffers);
        size_t len = ac_autocorr_length (n);

        for (int j = 0; j < n; j++) {
            src[j] = random() & 1;
        }

        ac_autocorr (buffers);

        for (int j = 0; j < n/2; j++) {
            CU_ASSERT (dst[len-j-1] == autocorr_in_point (src, len, j));
        }

        for (int j = 1; j < n/2; j++) {
            CU_ASSERT (dst[j-1] == autocorr_in_point (src, len, j));
        }

        ac_free (buffers);
    }
}

static CU_TestInfo fast_autocorr_tests[] = {
    { "Test parameters", test_params   },
    { "Fast vs naïve",   test_autocorr },
    CU_TEST_INFO_NULL
};

static CU_SuiteInfo suites[] = {
    { "Autocorr suite",  NULL, NULL, NULL, NULL, fast_autocorr_tests },
    CU_SUITE_INFO_NULL
};

int main () {
    int code;

    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf (stderr, "Failed to initialize registry: %s\n",
                 CU_get_error_msg ());
        return EXIT_FAILURE;
    }

    if (CU_register_suites (suites) != CUE_SUCCESS) {
        fprintf (stderr, "Failed to add suites: %s\n",
                 CU_get_error_msg ());
        CU_cleanup_registry ();
        return EXIT_FAILURE;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    code = (CU_get_number_of_tests_failed() == 0)? 0: EXIT_FAILURE;
    CU_cleanup_registry();

    return code;
}
