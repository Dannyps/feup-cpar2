#include "bitter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bitter* get_primes(long long int n)
{
    bitter* b = create_bitter(n);

    printf("Using %lld bytes to store %lld bits (%lld bits unused).\n", b->effectiveN, b->origN, b->effectiveN * 8 - b->origN);

    if (b == NULL) {
        return NULL;
    }

    printf("Setting all bits to one... ");
    for (long long int i = 0; i < n; i++) {
        setbit(b, i, 1);
    }

    printf("done.\n");

    for (long long int i = 2; i < sqrt(n); i++) {
        if (getbit(b, i) == 1) {
            long long int j, k = 0;
            for (j = i * i + i * k; j < n; j = i * i + i * k) {
                setbit(b, j, 0);
                k++;
            }
        }
    }

    return b;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Please provide a number! Use: %s <number> \n", argv[0]);
        return 1;
    }
    char print = 0;
    if (argc == 3) {
        print = atoi(argv[2]);
    }

    long long int n = atoll(argv[1]);

    if (n < 1) {
        fprintf(stderr,
            "The number that was provided is too small! Please provide an n > 0 (got %lld). \n", n);
        return 1;
    }

    bitter* b = get_primes(n);
    printf("Returned\n");
    if (b == NULL) {
        printf("Could not allocate RAM.\n");
        return 2;
    }

    unsigned int c = 0;
    for (long long int i = 2; i < n; i++) {
        if (getbit(b, i) == 1) {
            if (print)
                printf("%lld\t", i);
            c++;
        }
    }
    printf("\nFound %d prime numbers.\n", c);

    free(b->data);
    free(b);
    return 0;
}