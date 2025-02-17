#include <math.h>
#include <omp.h>
#include <papi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bitter.h"
#include "timer.c"

void handle_papi_error(int retval)
{
    printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
    exit(1);
}

void init_papi()
{
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT && retval < 0) {
        printf("PAPI library version mismatch!\n");
        exit(1);
    }
    if (retval < 0)
        handle_papi_error(retval);

    fprintf(stderr, "PAPI Version Number: MAJOR: %d,  MINOR: %d  REVISION: %d\n", PAPI_VERSION_MAJOR(retval), PAPI_VERSION_MINOR(retval), PAPI_VERSION_REVISION(retval));
}

bitter* get_primes(unsigned long long int n)
{
    bitter* b = create_bitter(n / 2 + 1);

    if (b == NULL) {
        return NULL;
    }

    fprintf(stderr,
        "Using %lld bytes to store %lld bits (%lld bits unused).\n",
        b->effectiveN, b->origN, b->effectiveN * 8 - b->origN);

    fprintf(stderr, "Setting all bits to one... ");

    fill(b, 1);

    fprintf(stderr, "done.\n");

    unsigned long sqrtn = sqrt(n) + 1;

    //#pragma omp parallel for
    for (unsigned long long i = 3; i <= sqrtn; i += 2) {
        // printf("[%d] Found %lld to be prime. %lld is a seed: %d.\n",
        // omp_get_thread_num(), i, i, i < sqrt(n));
        if (getbit(b, i / 2)) {
#pragma omp parallel for
            for (unsigned long long j = i * i; j <= n; j += 2 * i) {
                setbit(b, j / 2, 0);
                // printf("[%d] marking %lld as non prime.\n",
                // omp_get_thread_num(), j);
            }
        }
    }

    return b;
}

int main(int argc, char** argv)
{
    //Set up PAPI events
    int EventSet = PAPI_NULL;
    long long values[4] = { 0 }; // initialize array to 0
    int ret;

    ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT)
        fprintf(stderr, "[Error] PAPI_library_init\n");

    ret = PAPI_create_eventset(&EventSet);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_create_eventset\n");

    ret = PAPI_add_event(EventSet, PAPI_L1_DCM);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_L1_DCM\n");

    ret = PAPI_add_event(EventSet, PAPI_L2_DCM);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_L2_DCM\n");

    ret = PAPI_add_event(EventSet, PAPI_L1_DCH);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_L1_DCH\n");

    ret = PAPI_add_event(EventSet, PAPI_L2_DCH);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_L2_DCH\n");

    // Start counting hardware events
    ret = PAPI_start(EventSet);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] Start PAPI\n");

    struct timespec start2, start = getStart();

    if (argc < 2) {
        fprintf(stderr, "Please provide a number! Use: %s <number> \n",
            argv[0]);
        return 1;
    }
    char print = 0;
    if (argc == 3) {
        print = atoi(argv[2]);
    }

    long long int n = atoll(argv[1]);

    if (n < 1) {
        fprintf(stderr,
            "The number that was provided is too small! Please "
            "provide an n > "
            "0 (got %lld). \n",
            n);
        return 1;
    }

#ifdef OMP
    fprintf(stderr, "Running with OpenMP. Using %d threads.\n",
        omp_get_num_procs());
#endif

    bitter* b = get_primes(n);
    if (b == NULL) {
        fprintf(stderr, "Could not allocate RAM.\n");
        return 2;
    }

    double get_primes_time = getTime(start);
    fprintf(stderr, "get_primes(%lld) has returned. Counting... ", n);
    start2 = getStart();

    unsigned long long c = 0;
#pragma omp parallel for reduction(+ \
                                   : c)
    for (long long int i = 1; i < n; i += 2) {
        if (getbit(b, i / 2) == 1) {
            if (print)
                printf("%lld\t", i);
            c++;
        }
    }
    fprintf(stderr, "done. Found %lld prime numbers.\n", c);
    double count_time = getTime(start2);

    ret = PAPI_stop(EventSet, values);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI Stop\n");
    printf("L1 DCM: %lld \n", values[0]);
    printf("L2 DCM: %lld \n", values[1]);
    printf("L1 DCH: %lld \n", values[2]);
    printf("L2 DCH: %lld \n", values[3]);

    fprintf(stderr, "[TIME] get_primes:	%f s\n", get_primes_time);
    fprintf(stderr, "[TIME] count:		%f s\n", count_time);
    fprintf(stderr, "[TIME] TOTAL:		%f s\n", getTime(start));

    ret = PAPI_remove_event(EventSet, PAPI_L1_DCM);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_remove_event\n");

    ret = PAPI_remove_event(EventSet, PAPI_L2_DCM);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_remove_event\n");

    ret = PAPI_destroy_eventset(&EventSet);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI_destroy_eventset\n");
    delete_bitter(b);
    return EXIT_SUCCESS;
}