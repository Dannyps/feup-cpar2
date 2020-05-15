#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bitter.h"
#include "timer.c"

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
    fprintf(stderr, "[TIME] get_primes:	%f s\n", get_primes_time);
    fprintf(stderr, "[TIME] count:		%f s\n", count_time);
    fprintf(stderr, "[TIME] TOTAL:		%f s\n", getTime(start));
<<<<<<< Updated upstream
=======

    fprintf(stderr, "%f\t%f\t%f\t%lld\t%lld\n", get_primes_time, count_time, getTime(start), values[0], values[1]);

    ret = PAPI_remove_event(EventSet, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		fprintf(stderr, "[Error] PAPI_remove_event\n");

	ret = PAPI_remove_event(EventSet, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		fprintf(stderr, "[Error] PAPI_remove_event\n");

	ret = PAPI_destroy_eventset(&EventSet);
	if (ret != PAPI_OK)
		fprintf(stderr, "[Error] PAPI_destroy_eventset\n");
>>>>>>> Stashed changes
    delete_bitter(b);
    return EXIT_SUCCESS;
}