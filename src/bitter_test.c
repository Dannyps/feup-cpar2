#include "bitter.h"
#include <stdio.h>

int main()
{

    bitter* b = create_bitter(16);

    if (b == NULL) {
        printf("failure at create_bitter()");
        return -1;
    }

    printf("en = %lld\n", b->effectiveN);
    for (int i = 0; i < b->effectiveN; i++) {
        printf("0x%02x ", b->data[i]);
    }
    printf("\n");

    setbit(b, 2, 1);
    setbit(b, 3, 1);
    setbit(b, 4, 1);


    for (int i = 0; i < b->effectiveN; i++) {
        printf("0x%02x ", b->data[i]);
    }
    printf("\n");

    unsigned char bit = getbit(b, 5);
    printf("bit is %d\n", bit);
}
