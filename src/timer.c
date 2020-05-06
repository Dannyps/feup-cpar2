#include <stdlib.h>
#include <time.h>

struct timespec getStart()
{
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    return start;
}

double getTime(struct timespec start)
{
    struct timespec stop;
    clock_gettime(CLOCK_REALTIME, &stop);
    return (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)1000000000;
}