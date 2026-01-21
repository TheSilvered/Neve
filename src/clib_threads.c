#include "clib_threads.h"

#ifdef _WIN32

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    HANDLE handle = CreateThread(NULL, 0, routine, arg, 0, NULL);
    if (handle == NULL) {
        return false;
    }
    *thread = handle;
    return true;
}

ThreadID threadGetCurrID(void) {
    return GetCurrentThreadId();
}

bool threadIDEq(ThreadID id1, ThreadID id2) {
    return id1 == id2;
}

bool threadJoin(Thread thread, ThreadRet *status) {
    // The only possible return values are WAIT_FAILED and WAIT_OBJECT_0,
    // WAIT_ABANDONED is impossible because it is not a mutex and
    // WAIT_TIMEOUT is impossible because an infinite time is allowed
    if (WaitForSingleObject(thread, INFINITE) == WAIT_FAILED) {
        return false;
    }

    if (status != NULL && GetExitCodeThread(thread, status) == 0) {
        CloseHandle(thread);
        return false;
    }
    CloseHandle(thread);
    return true;
}

void threadExit(ThreadRet status) {
    ExitThread(status);
}

bool threadMutexInit(ThreadMutex *mutex) {
    InitializeSRWLock(mutex);
    return true;
}

bool threadMutexDestroy(ThreadMutex *mutex) {
    (void)mutex;
    return true; // no destructor for SRWLOCK
}

bool threadMutexLock(ThreadMutex *mutex) {
    AcquireSRWLockExclusive(mutex);
    return true;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    if (TryAcquireSRWLockExclusive(mutex)) {
        return ThreadLockResult_success;
    } else {
        return ThreadLockResult_busy;
    }
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    ReleaseSRWLockExclusive(mutex);
    return true;
}

#else

#include <errno.h>

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    return (errno = pthread_create(thread, NULL, routine, arg)) == 0;
}

ThreadID threadGetCurrID(void) {
    return pthread_self();
}

bool threadIDEq(ThreadID id1, ThreadID id2) {
    return pthread_equal(id1, id2);
}

bool threadJoin(Thread thread, ThreadRet *status) {
    return (errno = pthread_join(thread, status)) == 0;
}

void threadExit(ThreadRet status) {
    pthread_exit(status);
}

bool threadMutexInit(ThreadMutex *mutex) {
    return (errno = pthread_mutex_init(mutex, NULL)) == 0;
}

bool threadMutexDestroy(ThreadMutex *mutex) {
    return (errno = pthread_mutex_destroy(mutex)) == 0;
}

bool threadMutexLock(ThreadMutex *mutex) {
    return (errno = pthread_mutex_lock(mutex)) == 0;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    int error = pthread_mutex_trylock(mutex);
    switch (error) {
    case 0:
        return ThreadLockResult_success;
    case EBUSY:
        return ThreadLockResult_busy;
    default:
        errno = error;
        return ThreadLockResult_error;
    }
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    return (errno = pthread_mutex_unlock(mutex)) == 0;
}

#endif // !_WIN32
