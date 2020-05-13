#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned char* p;
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
    start = floor(rank * (n - 2) / size) + 2;
    end = floor((rank + 1) * (n - 2) / size) + 1;
    block_size = end - start + 1;

    // init memory
    p = malloc(block_size * sizeof(char));
    memset(p, 1, block_size);

    unsigned long long i = 2;
    while (i * i <= n) {
        unsigned long long ifm;
        if (start % i == 0)
            ifm = 0;
        else
            ifm = i - start % i;

        // from this first multiple, mark as non-prime every kth element
        for (unsigned long long j = ifm; j < block_size; j += i) {
            p[j] = 0;
        }

        if (rank == 0)
            p[i - 2] = 1;

        // set the next value of i to the smallest "true" number > current i
        // thread0 is in charge to find this value and broadcast it
        // note that this value is in thread 0 because we ensured that n/p > sqrt(n)
        if (rank == 0) {
            unsigned long long ni = i + 1;
            while (!p[ni - 2])
                ni = ni + 1;

            i = ni; //index to real value
        }
        // Now we found the next value of i, we must broadcast it to the other threads
        MPI_Bcast(&i, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    double get_primes_time = getTime(start_time);
    count_start_time = MPI_Wtime();

    local_sum = 0;
    for (i = 0; i < block_size; i++) {
        if (p[i])
            local_sum++;
    }
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
    double count_time = getTime(count_start_time);
    if (rank == 0) {
        fprintf(stderr, "done. Found %lld prime numbers.\n", global_sum);
        fprintf(stderr, "[TIME] get_primes:	%f s\n", get_primes_time);
        fprintf(stderr, "[TIME] count:		%f s\n", count_time);
        fprintf(stderr, "[TIME] TOTAL:		%f s\n", getTime(start_time));
    }

    free(p);

    MPI_Finalize();

    return 0;
}