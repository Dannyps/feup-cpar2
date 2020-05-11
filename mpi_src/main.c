#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int rank, size, erro;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    long long int n;

    printf("Hi from %d. We are %d.\n", rank, size);
    if (rank == 0) {
        if (argc < 2) {
            fprintf(stderr, "Please provide a number! Use: %s <number> \n",
                argv[0]);
            return 1;
        }
        char print = 0;
        if (argc == 3) {
            print = atoi(argv[2]);
        }

        n = atoll(argv[1]);
    }

    unsigned char* p = malloc(n / 2 + 1);

    unsigned long sqrtn = sqrt(n) + 1;

    memset(p, 1, n / 2 + 1);

    for (unsigned long long i = 3; i <= sqrtn; i += 2) {
        printf("Found %lld to be prime. %lld is a seed: %d.\n", i, i, i < sqrt(n));
        if (p[i / 2]) {
            for (unsigned long long j = i * i; j <= n; j += 2 * i) {
                p[j / 2] = 0;
                printf("Marking %lld as non prime.\n", j);
            }
        }
    }

    MPI_Finalize();

    return 0;
}