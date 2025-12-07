#include "nv_threads.h"
#include "nv_error.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    HANDLE handle = CreateThread(NULL, 0, routine, arg, 0, NULL);
    if (handle == NULL) {
        errSetErrno();
        return false;
    }
    if (thread != NULL) {
        *thread = handle;
    }
    return true;
}

ThreadID threadGetCurrID(void) {
    return GetCurrentThreadId();
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

bool threadMutexInit(ThreadMutex *mutex) {

}

void threadMutexDestroy(ThreadMutex *mutex) {

}

bool threadMutexLock(ThreadMutex *mutex) {
    return false;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    return ThreadLockResult_busy;
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    return false;
}
