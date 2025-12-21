#include "nv_time.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

uint64_t timeRelNs(void) {
    LARGE_INTEGER count;
    (void)QueryPerformanceCounter(&count);
    return count.QuadPart;
}

void timeSleep(uint64_t ns) {
    Sleep((DWORD)(ns / 1000000));
}
