#ifndef NV_THREADS_H_
#define NV_THREADS_H_

#include <stdbool.h>
#include "nv_utils.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HANDLE Thread;
typedef DWORD ThreadRet;
typedef DWORD ThreadID;
typedef struct ThreadMutex {
    SRWLOCK _lock;
    ThreadID _owner;
} ThreadMutex;

#else

#include <stdint.h>
#include <pthread.h>

typedef pthread_t Thread;
typedef pthread_t ThreadID;
typedef void *ThreadRet;
typedef pthread_mutex_t ThreadMutex;

#endif

#if __STDC_VERSION__ >= 201112L
    #if __STDC_VERSION__ < 202311L
        #define ThreadLocal _Thread_local
    #else
        #define ThreadLocal thread_local
    #endif
#elif defined(_MSC_VER) && !defined(__clang__)
#define ThreadLocal __declspec( thread )
#else
#define ThreadLocal __thread
#endif // !thread_local

typedef ThreadRet (*ThreadRoutine)(void *arg);

typedef enum ThreadLockResult {
    ThreadLockResult_success,
    ThreadLockResult_busy,
    ThreadLockResult_error
} ThreadLockResult;

// Create a thread
bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg);
// Get a unique identifier for the current thread
ThreadID threadGetCurrID(void);
// Wait until the thread exits
bool threadJoin(Thread thread, ThreadRet *status);
// Exit the current thread
NvNoreturn void threadExit(ThreadRet status);

// Initialize a mutex
bool threadMutexInit(ThreadMutex *mutex);
// Deinitialize a mutex
bool threadMutexDestroy(ThreadMutex *mutex);
// Wait for the mutex to unlock and then lock it
// Locking from the same thread again result in an error
bool threadMutexLock(ThreadMutex *mutex);
// Try locking a mutex, return `busy` immediately if it is already locked.
ThreadLockResult threadMutexTryLock(ThreadMutex *mutex);
// Unlock a mutex
bool threadMutexUnlock(ThreadMutex *mutex);

#endif // !NV_THREADS_H_
