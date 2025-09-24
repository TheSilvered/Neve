#include "nv_key_queue.h"

#define Q_INC_IDX(idx) (((idx) + 1) % keyQueueSize)

bool keyQueueEnq(KeyQueue *queue, TermKey key) {
    if (Q_INC_IDX(queue->back) == queue->front) {
        return false;
    }
    queue->keys[queue->back] = key;
    queue->back = Q_INC_IDX(queue->back);
    return true;
}

TermKey keyQueueDeq(KeyQueue *queue) {
    if (queue->front == queue->back) {
        return 0;
    }
    TermKey key = queue->keys[queue->front];
    queue->front = Q_INC_IDX(queue->front);
    return key;
}

