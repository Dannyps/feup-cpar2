#include <math.h>
#include <stdio.h>
#include <stdlib.h>

__uint8_t* get_primes(long long int n)
{
    __uint8_t* p = malloc(sizeof(__uint8_t) * n); //smallest data type possible

    if (p == NULL) {
        return NULL;
    }

    for (long long int i = 0; i < n; i++) {
        p[i] = 1;
    }

    for (long long int i = 2; i < sqrt(n); i++) {
        if (p[i] == 1) {
            long long int j, k = 0;
            for (j = i * i + i * k; j < n; j = i * i + i * k) {
                p[j] = 0;
                k++;
            }
        }
    }

    return p;
}

int main(int argc, char** argv)
{
    int print = 0;
    if (argc != 2) {
        fprintf(stderr, "Please provide a number! Use: %s <number> \n", argv[0]);
        return 1;
    }

    long long int n = atoll(argv[1]);

    if (n < 1) {
        fprintf(stderr,
            "The number that was provided is too small! Please provide an n > 0 (got %lld). \n", n);
        return 1;
    }

    __uint8_t* primes = get_primes(n);
    printf("Returned\n");
    if (primes == NULL) {
        printf("Could not allocate RAM.\n");
        return 2;
    }

    unsigned int c = 0;
    for (long long int i = 2; i < n; i++) {
        if (primes[i]) {
            if (print)
                printf("%lld\t", i);
            c++;
        }
    }
    printf("\nFound %d prime numbers.\n", c);

    free(primes);
    return 0;
}