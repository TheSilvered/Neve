#ifndef NV_KEY_QUEUE_H_
#define NV_KEY_QUEUE_H_

#include <stdint.h>
#include "nv_term.h"

#define keyQueueSize 128

typedef struct {
    uint16_t len, front, back;
    TermKey keys[keyQueueSize];
} KeyQueue;

// Add a key to the queue
void keyQueueEnq(KeyQueue *queue, TermKey key);
// Remove a key from the queue
TermKey keyQueueDeq(KeyQueue *queue);

#endif // !NV_KEY_QUEUE_H_

