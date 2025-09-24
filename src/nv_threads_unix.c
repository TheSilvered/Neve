#include "nv_threads.h"
#include "nv_error.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    int res = pthread_create(thread, NULL, routine, arg);
    if (res != 0) {
        errSetErrno();
    }
    return res == 0;
}

bool threadJoin(Thread thread, ThreadRet *status) {
    int res = pthread_join(thread, status);
    if (res != 0) {
        errSetErrno();
    }
    return res == 0;
}

void threadExit(ThreadRet status) {
    pthread_exit(status);
}

