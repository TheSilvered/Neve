#include "nv_time.h"

#include <time.h>
#include <errno.h>

uint64_t timeRelNs(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + (uint64_t)ts.tv_nsec;
}

void timeSleep(uint64_t ns) {
    struct timespec ts;
    ts.tv_sec = ns / 1000000000;
    ts.tv_nsec = ns % 1000000000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) { }
}
