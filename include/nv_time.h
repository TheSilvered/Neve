#ifndef NV_TIME_H_
#define NV_TIME_H_

#include <stdint.h>

// Get a relative timestamp in nanoseconds. The actual resolution may vary.
// Subsequent calls are guaranteed to always have a value greater than or equal
// to previous calls.
uint64_t timeRelNs(void);
// Sleep by `ns` nanoseconds.
void timeSleep(uint64_t ns);

#endif // !NV_TIME_H_
