#include "nv_key_queue.h"

void keyQueueEnq(KeyQueue *queue, TermKey key) {
    if (queue->len == keyQueueSize) {
        return;
    }
    queue->keys[queue->back] = key;
    queue->back = (queue->back + 1) % keyQueueSize;
    queue->len++;
}

TermKey keyQueueDeq(KeyQueue *queue) {
    if (queue->len == 0) {
        return 0;
    }
    TermKey key = queue->keys[queue->front];
}
