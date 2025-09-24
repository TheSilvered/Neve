#include "nv_threads.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    return pthread_create(
        thread,
        NULL,
        (void *(*)(void *))(void *)routine, // safe since ThreadRet is uintptr_t
        arg) == 0;
}

bool threadJoin(Thread thread, ThreadRet *status) {
    return pthread_join(thread, (void **)status);
}

void threadExit(ThreadRet status) {
    pthread_exit((void *)status);
}

