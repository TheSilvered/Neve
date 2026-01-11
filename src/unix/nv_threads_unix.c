#include <errno.h>
#include "nv_threads.h"

bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg) {
    int res = pthread_create(thread, NULL, routine, arg);
    return res == 0;
}

ThreadID threadGetCurrID(void) {
    return pthread_self();
}

bool threadIDEq(ThreadID id1, ThreadID id2) {
    return pthread_equal(id1, id2);
}

bool threadJoin(Thread thread, ThreadRet *status) {
    int res = pthread_join(thread, status);
    return res == 0;
}

void threadExit(ThreadRet status) {
    pthread_exit(status);
}

bool threadMutexInit(ThreadMutex *mutex) {
    if (pthread_mutex_init(&mutex->_mutex, NULL) != 0) {
        return false;
    }
    mutex->_locked = false;
    return true;
}

bool threadMutexDestroy(ThreadMutex *mutex) {
    if (pthread_mutex_destroy(&mutex->_mutex) != 0) {
        return false;
    }
    mutex->_locked = false;
    return true;
}

bool threadMutexLock(ThreadMutex *mutex) {
    ThreadID self = threadGetCurrID();
    if (mutex->_locked && pthread_equal(self, mutex->_owner)) {
        errno = EDEADLK;
        return false;
    }

    if (pthread_mutex_lock(&mutex->_mutex) != 0) {
        return false;
    }
    mutex->_owner = self;
    mutex->_locked = true;
    return true;
}

ThreadLockResult threadMutexTryLock(ThreadMutex *mutex) {
    ThreadID self = threadGetCurrID();
    if (mutex->_locked && pthread_equal(self, mutex->_owner)) {
        errno = EDEADLK;
        return ThreadLockResult_error;
    }

    if (pthread_mutex_trylock(&mutex->_mutex) == 0) {
        mutex->_owner = self;
        mutex->_locked = true;
        return ThreadLockResult_success;
    } else if (errno == EBUSY) {
        return ThreadLockResult_busy;
    } else {
        return ThreadLockResult_error;
    }
}

bool threadMutexUnlock(ThreadMutex *mutex) {
    if (!mutex->_locked || !pthread_equal(threadGetCurrID(), mutex->_owner)) {
        errno = EPERM;
        return false;
    }
    mutex->_locked = false;
    if (pthread_mutex_unlock(&mutex->_mutex) != 0) {
        return false;
    }
    return true;
}
