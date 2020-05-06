#include <stdio.h>
#include <stdlib.h>

/** @struct bitter
  *  Holds the relevant data
  *
  *  @var bitter::data
  *    The data.
  *  @var whatsit::origN
  *    Original number of bits to be stored.
  *  @var whatsit::effectiveN
  *    The effective number of bytes that werer stored.
  */
typedef struct {
    __uint8_t* data;
    unsigned long long origN, effectiveN;
} bitter;

/**
 * @brief Create a bitter object
 * 
 * @param n number of bits to allocate
 * @return bitter* or NULL on malloc failure 
 * @warn PLEASE CHECK MY RETURN VALUE
 */
bitter* create_bitter(unsigned long long n);

int fill(bitter* b, __uint128_t val);

__int8_t setbit(bitter* b, unsigned long long n, __uint128_t val);

__int8_t getbit(bitter* b, unsigned long long n);

