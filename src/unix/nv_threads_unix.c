#include <errno.h>
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

bool threadMutexInit(ThreadMutex *mutex) {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        errSetErrno();
        return false;
    }
    (void)pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (pthread_mutex_init(mutex, &attr) != 0) {
        errSetErrno();
        return false;
    }
    (void)pthread_mutexattr_destroy(&attr);
    return true;
}

void threadMutexDestroy(ThreadMutex *mutex) {
    (void)pthread_mutex_destroy(mutex);
}

bool threadMutexLock(ThreadMutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        errSetErrno();
        return false;
    }
    return true;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    if (pthread_mutex_trylock(mutex) == 0) {
        return ThreadLockResult_success;
    } else if (errno == EBUSY) {
        return ThreadLockResult_busy;
    } else {
        errSetErrno();
        return ThreadLockResult_error;
    }
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        errSetErrno();
        return false;
    }
    return true;
}
