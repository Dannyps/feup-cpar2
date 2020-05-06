#include "bitter.h"
#include <math.h>
#include <omp.h>
#include <stdio.h>

bitter* create_bitter(unsigned long long n)
{
    bitter* b = malloc(sizeof(bitter));
    b->origN = n;
    b->effectiveN = ceil(n / 8.0);

    fprintf(stderr, "Trying to malloc %llu bytes... ", b->effectiveN);
    b->data = malloc(b->effectiveN);
    if (b->data == NULL) {
        return NULL;
    }
    return b;
}

int fill(bitter* b, __uint128_t val)
{
    if (val == 1) {
        for (unsigned long i = 0; i < b->effectiveN; i++) {
            b->data[i] = 0xFF;
        }
    } else if (val == 0) {
        for (unsigned long i = 0; i < b->effectiveN; i++) {
            b->data[i] = 0x0;
        }
    } else {
        return -2; // unsupported val
    }

    return 0;
}

__int8_t setbit(bitter* b, unsigned long long n, __uint128_t val)
{
    if (n >= b->origN) {
        return -1; // accessing inaccessible bit;
    }

    register unsigned long byte = n / 8;
    register unsigned char offset = n % 8; // aka bit

    if (val == 1) {
        //        printf("setting %dth bit of byte %d to %d\n", offset, byte, val);
        if (offset == 0) {
#pragma omp atomic
            b->data[byte] |= 1UL;
        } else {
#pragma omp atomic
            b->data[byte] |= 1UL << offset;
        }
    } else if (val == 0) {
#pragma omp atomic
        b->data[byte] &= ~(1UL << offset);
    } else {
        return -2; // unsupported val
    }
    return 0;
}

__int8_t getbit(bitter* b, unsigned long long n)
{
    if (n >= b->origN) {
        return -1; // accessing inaccessible bit;
    }

    register unsigned long byte = n / 8;
    register unsigned char offset = n % 8; // aka bit

    return (b->data[byte] >> offset) & 1;
}
