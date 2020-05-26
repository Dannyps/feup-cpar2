#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_FIRST 3 /* first odd prime number */
#define BLOCK_STEP 2 /* loop step to iterate only for odd numbers */

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BLOCK_LOW(id, p, n) \
    ((id) * (n) / (p) / BLOCK_STEP)

#define BLOCK_HIGH(id, p, n) \
    (BLOCK_LOW((id) + 1, p, n) - 1)

#define BLOCK_SIZE(id, p, n) \
    (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW((id), p, n))

#define BLOCK_OWNER(index, p, n) \
    (((p) * ((index) + 1) - 1) / (n))

#define BLOCK_VALUE_TO_INDEX(val, id, p, n) \
    (val - BLOCK_FIRST) / BLOCK_STEP - BLOCK_LOW(id, p, n - 1)

double getTime(double time)
{
    return MPI_Wtime() - time;
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank, size, erro;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    unsigned long long n;
    unsigned char print;
    unsigned long long start;
    unsigned long long end;
    unsigned long long block_size;
    unsigned long long local_sum;
    unsigned long long global_sum;

    double count_start_time, start_time = MPI_Wtime();

    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "[Error] Please provide a number! Use: %s <number> \n",
                argv[0]);
        }
        MPI_Finalize();
        return 0;
    }
    if (argc == 3) {
        print = atoi(argv[2]);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    n = atoll(argv[1]);

    if ((n / size <= sqrt(n))) {
        if (0 == rank) {
            fprintf(stderr, "[Error] There are too many processes! \n");
        }
        MPI_Finalize();
        exit(0);
    }

    // The workload must be split throughout the processes into equal size contiguous blocks
    // for each process, get the values of the first and last elems and the number of elems
    start = BLOCK_FIRST + BLOCK_LOW(rank, size, n - 1) * BLOCK_STEP;
    end = BLOCK_FIRST + BLOCK_HIGH(rank, size, n - 1) * BLOCK_STEP;
    block_size = BLOCK_SIZE(rank, size, n - 1);

    // find all primes from 2 to sqrtn

    //square root of n
    unsigned sqrtn = sqrt(n);
    char* primes = malloc((sqrtn + 1) * sizeof(char));
    memset(primes, 1, (sqrtn + 1));
    unsigned pm;
    for (pm = 2; pm <= sqrt(n); pm += 2) {
        primes[pm] = 0;
    }
    // the currently under analysis prime
    unsigned prime;
    for (prime = 3; prime <= sqrtn; prime += 2) {
        if (primes[prime] == 0)
            continue;

        for (pm = prime << 1; pm <= sqrtn; pm += prime) {
            //printf("setting %d as non prime from seed %d\n", pm, prime);
            primes[pm] = 0;
        }
    }

    // for (int i = 0; rank == 0 && i < sqrtn + 1; i++) {
    //     printf("primes[%d] = %d\n", i, primes[i]);
    // }

    MPI_Barrier(MPI_COMM_WORLD);

    /* 
     * allocate this process' share of the array 
     */

    char* marked = malloc(block_size * sizeof(char));
    memset(marked, 1, block_size);

    if (marked == NULL) {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    printf("%d/%d got start=%lld, end=%lld, block_size=%lld\n", rank, size, start, end, block_size);

    unsigned first;
    for (prime = 3; prime <= sqrtn; prime++) {
        if (primes[prime] == 0)
            continue;
        if (prime * prime > start) {
            printf("%d/%d got start=%lld, prime=%ld A ", rank, size, start, prime);
            first = prime * prime;
        } else {
            if (start % prime == 0) {
                printf("%d/%d got start=%lld, prime=%ld B ", rank, size, start, prime);
                first = start;
            } else {
                printf("%d/%d got start=%lld, prime=%ld C ", rank, size, start, prime);
                first = prime - (start % prime) + start;
            }
        }
        printf("first=%ld\n", first);

        if ((first + prime) % 2) // is odd
            first += prime;

        unsigned long first_value_index = (first - 3) / 2 - BLOCK_LOW(rank, size, n - 1);

        for (unsigned long i = first; i <= end; i += prime*2) {
            marked[first_value_index] = 0;
            first_value_index += prime;
        }
    }

    double get_primes_time = getTime(start_time);
    count_start_time = MPI_Wtime();
    /* 
     * count the number of prime numbers found on this process 
     */

    local_sum = 0;
    for (unsigned long i = 0; i < block_size; i++) {
        if (marked[i]) {
            //printf("[%d] prime: %lld\n", rank, start + i * 2);
            local_sum++;
        }
    }
    if (rank == 0) {
        local_sum++; // number 2
    }

    double count_time = getTime(count_start_time);
    //fprintf(stderr, "[%d][TIME] count_time:	%f s\n", rank, count_time);
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        fprintf(stderr, "done. Found %lld prime numbers.\n", global_sum);
        fprintf(stderr, "[TIME] get_primes:	%f s\n", get_primes_time);
        fprintf(stderr, "[TIME] count:		%f s\n", count_time);
        fprintf(stderr, "[TIME] TOTAL:		%f s\n", getTime(start_time));
        fprintf(stderr, "%f\t%f\t%f\n", get_primes_time, count_time, getTime(start_time));
    }

    free(marked);
    free(primes);

    MPI_Finalize();

    return 0;
}