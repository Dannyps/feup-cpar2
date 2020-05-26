#include <math.h>
#include <omp.h>
#include <papi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "timer.c"

#define BLOCK_LOW(id, p, n) \
    ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) \
    (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) \
    (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)

void naive_block_decomposition(unsigned long long int n)
{
    /** Starting prime for all threads */
    unsigned long long k = 2;
    unsigned long long count = 0;
    unsigned long long prime_index = 0;
    #pragma omp parallel
    {
        /** The thread identifier */
        int id = omp_get_thread_num();
        /** Number of threads running */
        int num_threads = omp_get_num_threads();
        /** This thread lower allocated number */
        int lower_num = 2 + BLOCK_LOW(id, num_threads, n);
        /** This thread higher allocated number. If it exceeds `n`, adjust it */
        int higher_num = 2 + BLOCK_HIGH(id, num_threads, n);
        if (higher_num > n)
            higher_num = n;
        /** This thread block size, i.e., how many numbers it will process */
        int block_size = higher_num - lower_num + 1;

        //printf("Hello from thread %d | Start: %d | End: %d\n", id, lower_num, higher_num);

        /**
         * Allocate memory for this thread's block of prime numbers.
         * Memory block is initialized to 0. Positions marked as 1 are non-prime numbers
         */
        uint8_t* my_block = (uint8_t*)calloc(1, block_size);

        do {
            /** 
             * Compute the index where this thread should start marking numbers.
             * 
             * Each thread must mark numbers between: k^2 and n
             * 
             * Therefore, if the lower number is less than k, we compute the index for k*k.
             * If this block is on the desired range, [ k^2, n], then check if the lower number
             * of this block is multiple of `k`. If so, we start at index 0. Otherwise, we
             * need to find the first index that maps to a number multiple of `k`
             */
            int first_index = 0;

            if (lower_num < k * k) {
                first_index = k * k - lower_num;
            } else if (lower_num % k != 0) {
                do {
                    first_index++;
                } while ((lower_num + first_index) % k != 0);
            }

            //printf("Hello from thread %d. My starting index is %d\n", id, first_index);

            /**
             * Mark all multiples of `k` in this thread's block of numbers
             */
            for (int i = first_index; i < block_size; i += k) {
                my_block[i] = 1;
                //printf("Hello from thread %d. Marked %d as non-prime\n", id, lower_num + i);
            }

            /** 
             * Barrier for waiting for all threads before updating the value of `k`
             * Only thread 0 can update its value
             */
            #pragma omp barrier
            if (id == 0) {
                // find closest unmarked number (the next prime)
                while (my_block[++prime_index])
                    ;
                k = prime_index + 2;
                //printf("Found next prime: %d\n", k);
            }

        /** Another barrier to ensure all threads reach the end of this loop iteration */
        #pragma omp barrier
        } while (k * k <= n);

        int local_count = 0;
        for (int i = 0; i < block_size; i++) {
            if (my_block[i] == 0)
                local_count++;
        }
        free(my_block);
        #pragma omp atomic
        count += local_count;
    }

    printf("Done!\n");
    printf("Found %d primes\n", count);
}

void block_decomposition_no_evens(unsigned long long int n)
{
    /** Starting prime for all threads */
    unsigned long long k = 3;
    unsigned long long count = 1;
    unsigned long long prime_index = 0;
    #pragma omp parallel num_threads(4)
    {
        /** The thread identifier */
        int id = omp_get_thread_num();
        /** Number of threads running */
        int num_threads = omp_get_num_threads();
        /** This thread lower allocated number */
        int lower_num = 2 + BLOCK_LOW(id, num_threads, n);
        /** This thread higher allocated number. If it exceeds `n`, adjust it */
        int higher_num = 2 + BLOCK_HIGH(id, num_threads, n);
        if (higher_num > n)
            higher_num = n;
        /** This thread block size, i.e., how many numbers it will process */
        int block_size = higher_num - lower_num + 1;
        /** 
         * Adjust the lower and higher numbers (and block size) for this block to discard even numbers
         */
        if(lower_num % 2 == 0) {
            /** if lower is even, then increment it */
            lower_num++;

            if(higher_num % 2 == 0) {
                /** 
                 * If both are even, then the extremes are removed (-2).
                 * Then we cut the block size in half to discard remaining evens
                 */
                block_size = ceil((block_size - 2)/2);
                higher_num--;
            } else {
                /**
                 * If we reach here, then we have a even block size
                 * It's enough to compute half of the block size
                 */
                block_size = block_size / 2;
            }
        }
        else if (higher_num % 2 == 0) {
            /** if higher number is even, decrement it */
            higher_num--;
            /**
             * The block is even because lower_num is odd and higher_num is even
             * It's enough to compute half of the block size
             */
            block_size = block_size / 2;
        }
        else {
            /** If both extremes values are odd, we have odd block size */
            block_size = ceil(block_size/2);
        }
        

        printf("Hello from thread %d | Start: %d | End: %d\n", id, lower_num, higher_num);

        /**
         * Allocate memory for this thread's block of prime numbers.
         * Memory block is initialized to 0. Positions marked as 1 are non-prime numbers
         */
        uint8_t* my_block = (uint8_t*)calloc(1, block_size);

        do {
            /** 
             * Compute the index where this thread should start marking numbers.
             * 
             * Each thread must mark numbers between: k^2 and n
             * 
             * Therefore, if the lower number is less than k, we compute the index for k*k.
             * If this block is on the desired range, [ k^2, n], then check if the lower number
             * of this block is multiple of `k`. If so, we start at index 0. Otherwise, we
             * need to find the first index that maps to a number multiple of `k`
             */
            int first_index = 0;

            if (lower_num < k * k) {
                first_index = (k * k - lower_num)/2;
            } else if (lower_num % k != 0) {
                do {
                    first_index++;
                } while ((lower_num + ( 2 * first_index)) % k != 0);
            }

            //printf("Hello from thread %d. My starting index is %d\n", id, first_index);

            /**
             * Mark all multiples of `k` in this thread's block of numbers
             */
            for (int i = first_index; i < block_size; i += k) {
                my_block[i] = 1;
                //printf("Hello from thread %d. Marked %d as non-prime\n", id, lower_num + i);
            }

            /** 
             * Barrier for waiting for all threads before updating the value of `k`
             * Only thread 0 can update its value
             */
            #pragma omp barrier
            if (id == 0) {
                // find closest unmarked number (the next prime)
                while (my_block[++prime_index])
                    ;
                k = prime_index * 2 + 3;
                //printf("Found next prime: %d\n", k);
            }

        /** Another barrier to ensure all threads reach the end of this loop iteration */
        #pragma omp barrier
        } while (k * k <= n);

        int local_count = 0;
        for (int i = 0; i < block_size; i++) {
            if (my_block[i] == 0)
                local_count++;
        }
        free(my_block);
        #pragma omp atomic
        count += local_count;
    }

    printf("Done!\n");
    printf("Found %d primes\n", count);
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

    //naive_block_decomposition(n);
    block_decomposition_no_evens(n);

    ret = PAPI_stop(EventSet, values);
    if (ret != PAPI_OK)
        fprintf(stderr, "[Error] PAPI Stop\n");
    printf("L1 DCM: %lld \n", values[0]);
    printf("L2 DCM: %lld \n", values[1]);
    printf("L1 DCH: %lld \n", values[2]);
    printf("L2 DCH: %lld \n", values[3]);

    //fprintf(stderr, "[TIME] get_primes:	%f s\n", get_primes_time);
    //fprintf(stderr, "[TIME] count:		%f s\n", count_time);
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

    return EXIT_SUCCESS;
}