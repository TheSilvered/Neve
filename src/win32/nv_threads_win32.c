#include "nv_threads.h"
#include "nv_error.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    if (CreateThread(NULL, 0, routine, arg, 0, thread) == NULL) {
        errSetErrno();
        return false;
    }
    return true;
}

Thread threadGetSelf(void) {
    return GetCurrentThreadId();
}

bool threadJoin(Thread thread, ThreadRet *status) {
    HANDLE threadHandle = OpenThread(NULL, false, thread);
    // The only possible return values are WAIT_FAILED and WAIT_OBJECT_0,
    // WAIT_ABANDONED is impossible because it is not a mutex and
    // WAIT_TIMEOUT is impossible because an infinite time is allowed
    if (WaitForSingleObject(threadHandle, INFINITE) == WAIT_FAILED) {
        errSetErrno();
        return false;
    }
    if (status == NULL) {
        return true;
    }

    if (GetExitCodeThread(threadHandle, status) == 0) {
        errSetErrno();
        return false;
    }
    return true;
}

void threadExit(ThreadRet status) {
    ExitThread(status);
}

bool threadMutexInit(ThreadMutex *mutex) {
    return false;
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
