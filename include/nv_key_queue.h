#ifndef NV_KEY_QUEUE_H_
#define NV_KEY_QUEUE_H_

#include <stdint.h>
#include "nv_term.h"

#define keyQueueSize 128

typedef struct {
    uint16_t front, back;
    TermKey keys[keyQueueSize];
} KeyQueue;

// Add a key to the queue
// To be called only by the input thread
bool keyQueueEnq(KeyQueue *queue, TermKey key);
// Remove a key from the queue
// To be calle only by the main thread
TermKey keyQueueDeq(KeyQueue *queue);

#endif // !NV_KEY_QUEUE_H_
