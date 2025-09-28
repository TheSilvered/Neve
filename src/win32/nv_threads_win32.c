#include "nv_threads.h"
#include "nv_error.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    HANDLE newThread = CreateThread(NULL, 0, routine, arg, 0, NULL);

    if (newThread == NULL) {
        errSetErrno();
        return false;
    }
    if (thread != NULL) {
        *thread = newThread;
    }
    return true;
}

bool threadJoin(Thread thread, ThreadRet *status) {
    // The only possible return values are WAIT_FAILED and WAIT_OBJECT_0,
    // WAIT_ABANDONED is impossible because it is not a mutex and
    // WAIT_TIMEOUT is impossible because an infinite time is allowed
    if (WaitForSingleObject(thread, INFINITE) == WAIT_FAILED) {
        errSetErrno();
        return false;
    }
    if (status == NULL) {
        return true;
    }

    if (GetExitCodeThread(thread, status) == 0) {
        errSetErrno();
        return false;
    }
    return true;
}

void threadExit(ThreadRet status) {
    ExitThread(status);
}

