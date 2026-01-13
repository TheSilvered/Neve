/*
Cross-platform threading.

License: MIT license, see the bottom of the file.
*/

#ifndef CLIB_THREADS_H_
#define CLIB_THREADS_H_

#include <stdbool.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HANDLE Thread;
typedef DWORD ThreadRet;
typedef DWORD ThreadID;
typedef SRWLOCK ThreadMutex;

// Statically initialize a mutex.
#define ThreadMutexInitializer SRWLOCK_INIT

#else

#include <stdint.h>
#include <pthread.h>

typedef pthread_t Thread;
typedef pthread_t ThreadID;
typedef void *ThreadRet;
typedef pthread_mutex_t ThreadMutex;

// Statically initialize a mutex.
#define ThreadMutexInitializer PTHREAD_MUTEX_INITIALIZER

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

// Create a thread. `thread` cannot be `NULL`.
// On failure `errno` is set on POSIX and `GetLastError()` on Windows.
bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg);
// Get a unique identifier for the current thread.
ThreadID threadGetCurrID(void);
// Check if two `ThreadID`s are equal.
bool threadIDEq(ThreadID id1, ThreadID id2);
// Wait until the thread exits. `status` may be `NULL`.
// On failure `errno` is set on POSIX and `GetLastError()` on Windows.
bool threadJoin(Thread thread, ThreadRet *status);
// Exit the current thread.
void threadExit(ThreadRet status);

// Initialize a mutex.
bool threadMutexInit(ThreadMutex *mutex);
// Deinitialize a mutex.
bool threadMutexDestroy(ThreadMutex *mutex);
// Wait for the mutex to unlock and then lock it. Locking from the same thread
// again is undefined behaviour (likely results in a deadlock).
// On failure `errno` is set on POSIX and `GetLastError()` on Windows.
bool threadMutexLock(ThreadMutex *mutex);
// Try locking a mutex, return `busy` immediately if it is already locked.
// On failure `errno` is set on POSIX and `GetLastError()` on Windows.
// `ThreadLockResult_busy` is not considered a failure.
ThreadLockResult threadMutexTryLock(ThreadMutex *mutex);
// Unlock a mutex.
// On failure `errno` is set on POSIX and `GetLastError()` on Windows.
bool threadMutexUnlock(ThreadMutex *mutex);

#endif // !CLIB_THREADS_H_

/*
MIT License

Copyright (c) 2026 Davide Taffarello

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
