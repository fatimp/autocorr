#include <CUnit/CUError.h>
#include <CUnit/TestDB.h>
#include <CUnit/TestRun.h>
#include <CUnit/Basic.h>

#include <autocorr.h>
#include <consts.h>

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
    for (int i = 0; i < MAX_STEPS; i++) {
        uint64_t length = 1 << (i + 1);
        uint64_t omega = _omegas[i];
        uint64_t inv_omega = _inv_omegas[i];
        uint64_t inv_length = _inv_lengths[i];

        CU_ASSERT ((length & (length - 1)) == 0);
        CU_ASSERT ((omega * inv_omega) % P == 1);
        CU_ASSERT ((length * inv_length) % P == 1);
        CU_ASSERT (expt(omega, length, 1, P) == 1);
    }
}

int fast_ac_ok (uint64_t *dst, uint64_t *src, int n, size_t len) {
    for (int i = 0; i < n/2; i++) {
        if (dst[len-i-1] != autocorr_in_point (src, len, i)) {
            return 0;
        }
    }

    for (int i = 1; i < n/2; i++) {
        if (dst[i-1] != autocorr_in_point (src, len, i)) {
            return 0;
        }
    }

    return 1;
}

int all_ones_ac_ok (uint64_t *dst, int n, size_t len) {
    for (int i = 0; i < n/2; i++) {
        if (dst[len-i-1] != n - i) {
            return 0;
        }
    }

    for (int i = 0; i < n/2; i++) {
        if (dst[i] != n - i - 1) {
            return 0;
        }
    }

    return 1;
}

static void test_autocorr () {
    for (int i = 0; i < 1000; i++) {
        size_t n = 3 + arc4random_uniform (5000);
        struct ac_buffers *buffers = ac_alloc (n);
        uint64_t *src = ac_get_src (buffers);
        uint64_t *dst = ac_get_dst (buffers);
        size_t len = ac_autocorr_length (n);

        for (int j = 0; j < n; j++) {
            src[j] = arc4random() & 1;
        }

        ac_autocorr (buffers);
        CU_ASSERT (fast_ac_ok (dst, src, n, len));
        ac_free (buffers);
    }
}

static void test_all_ones () {
    for (int i = 0; i < 200; i++) {
        size_t n = 3 + arc4random_uniform (1048576-3);
        struct ac_buffers *buffers = ac_alloc (n);
        uint64_t *src = ac_get_src (buffers);
        uint64_t *dst = ac_get_dst (buffers);
        size_t len = ac_autocorr_length (n);

        for (int j = 0; j < n; j++) {
            src[j] = 1;
        }

        ac_autocorr (buffers);
        CU_ASSERT (all_ones_ac_ok (dst, n, len));
        ac_free (buffers);
    }
}

static CU_TestInfo fast_autocorr_tests[] = {
    { "Test parameters", test_params   },
    { "Fast vs naïve",   test_autocorr },
    { "All ones",        test_all_ones },
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
