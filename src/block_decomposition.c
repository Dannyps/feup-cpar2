#include <math.h>
#include <omp.h>
#include <papi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bitter.h"
#include "timer.c"

#define BLOCK_LOW(id, p, n) \
    ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) \
    (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) \
    (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)

void own_sieving_block_decomposition(uint64_t n)
{
    /** Compute a list of primes in range 2..sqrt(n) */
    uint64_t k = 2;
    uint64_t prime_index = 0;
    uint64_t sqrt_n = ceil(sqrt((double)n));
    bitter* pre_seived = create_bitter(sqrt_n);
    fill(pre_seived, 0);

    do {
        for(uint64_t i = k*k; i <= sqrt_n; i += k) {
            // mark as non-prime (index 0 maps to 2, index 1 to 3, ...)
            setbit(pre_seived, i - 2, 1);
        }
        // find the next smallest prime
        do { prime_index++; } while(getbit(pre_seived, prime_index) == 1);
        k = prime_index + 2;
    } while(k*k <= sqrt_n);

    /*for(int i = 0; i < sqrt_n; i++)
        if(pre_seived[i] == 0)
            printf("Seeded prime: %ld\n", i + 2);*/

    /** Starting prime for all threads */
    k = 3;
    prime_index = 1;
    uint64_t count = 1; // count with the only even number: 2

    #pragma omp parallel firstprivate(k, prime_index)
    {
        /** The thread identifier */
        uint8_t id = omp_get_thread_num();
        /** Number of threads running */
        uint8_t num_threads = omp_get_num_threads();
        /** This thread lower allocated number */
        uint64_t lower_num = 2 + BLOCK_LOW(id, num_threads, n);
        /** This thread higher allocated number. If it exceeds `n`, adjust it */
        uint64_t higher_num = 2 + BLOCK_HIGH(id, num_threads, n);
        if (higher_num > n)
            higher_num = n;
        /** This thread block size, i.e., how many numbers it will process */
        uint64_t block_size = higher_num - lower_num + 1;
        //printf("Hello from thread %d | Start: %ld | End: %ld | Block size: %ld\n", id, lower_num, higher_num, block_size);
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
                block_size = ceil((block_size - 2)/2.0);
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
            block_size = ceil(block_size/2.0);
        }
        
        //printf("Hello from thread %d | Start: %ld | End: %ld | Block size: %ld\n", id, lower_num, higher_num, block_size);

        /**
         * Allocate memory for this thread's block of prime numbers.
         * Memory block is initialized to 0. Positions marked as 1 are non-prime numbers
         */
        bitter* my_block = create_bitter(block_size);
        fill(my_block, 0);

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
            uint64_t first_index = 0;

            if (lower_num < k * k) {
                first_index = (k * k - lower_num)/2;
            } else if (lower_num % k != 0) {
                do {
                    first_index++;
                } while ((lower_num + ( 2 * first_index)) % k != 0);
            }

            //printf("Hello from thread %d. My starting index is %ld\n", id, first_index);

            /**
             * Mark all multiples of `k` in this thread's block of numbers
             */
            for (uint64_t i = first_index; i < block_size; i += k) {
                setbit(my_block, i, 1);
                //printf("Hello from thread %d. Marked %ld as non-prime\n", id, lower_num + i*2);
            }
            /** 
             * Barrier for waiting for all threads before updating the value of `k`
             * Only thread 0 can update its value
             */
            while (getbit(pre_seived, ++prime_index));
            k = prime_index + 2;
            //printf("Hello from thead %d. Next prime seed: %ld\n", id, k);
        } while (k * k <= n);

        uint64_t local_count = 0;
        for (uint64_t i = 0; i < block_size; i++) {
            if (getbit(my_block, i) == 0) {
                local_count++;
                //printf("Hello from thread %d. Number %ld is prime\n", id, lower_num + i*2);
            }
        }
        free(my_block);
        #pragma omp atomic
        count += local_count;
    }

    delete_bitter(pre_seived);

    printf("Done!\n");
    printf("Found %ld primes\n", count);
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

    struct timespec start = getStart();

    if (argc < 2) {
        fprintf(stderr, "Please provide a number! Use: %s <number> \n",
            argv[0]);
        return 1;
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

    own_sieving_block_decomposition(n);

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