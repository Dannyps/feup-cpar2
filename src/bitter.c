#include "bitter.h"
#include <math.h>
#include <stdio.h>

bitter* create_bitter(unsigned long long n)
{
    bitter* b = malloc(sizeof(bitter));
    b->origN = n;
    b->effectiveN = ceil(n / 8.0);

    b->data = calloc(b->effectiveN, 1);
    return b;
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
            b->data[byte] |= 1UL;
        } else {
            b->data[byte] |= 1UL << offset;
        }
    } else if (val == 0) {
        b->data[byte] &= ~(1UL << offset);
    } else {
        return -2; // unsupported val
    }
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
