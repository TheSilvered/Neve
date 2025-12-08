#include <assert.h>
#include "nv_threads.h"

#ifdef NV_THREADS_NO_ERROR
#define errSetErrno()
#else
#include "nv_error.h"
#endif

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
    InitializeSRWLock(&mutex->_lock);
    mutex->_owner = 0;
    return true;
}

bool threadMutexDestroy(ThreadMutex *mutex) {
    if (mutex->_owner != 0) {
        SetLastError(ERROR_BUSY);
        errSetErrno();
        return false;
    }
    return true; // no destructor for SRWLOCK
}

bool threadMutexLock(ThreadMutex *mutex) {
    DWORD id = threadGetCurrID();
    if (mutex->_owner == id) {
        SetLastError(ERROR_POSSIBLE_DEADLOCK);
        errSetErrno();
        return false;
    }
    AcquireSRWLockExclusive(&mutex->_lock);
    mutex->_owner = id;
    return true;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    DWORD id = threadGetCurrID();
    if (mutex->_owner == id) {
        SetLastError(ERROR_POSSIBLE_DEADLOCK);
        errSetErrno();
        return ThreadLockResult_error;
    } else if (TryAcquireSRWLockExclusive(&mutex->_lock) == TRUE) {
        mutex->_owner = id;
        return ThreadLockResult_success;
    } else {
        return ThreadLockResult_busy;
    }
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    if (mutex->_owner != threadGetCurrID()) {
        SetLastError(ERROR_NOT_OWNER);
        errSetErrno();
        return false;
    }
    mutex->_owner = 0;
    ReleaseSRWLockExclusive(&mutex->_lock);
    return true;
}
