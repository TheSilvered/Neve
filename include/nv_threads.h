#ifndef NV_THREADS_H_
#define NV_THREADS_H_

#include <stdbool.h>
#include "nv_utils.h"

#ifdef _WIN32

#include <windows.h>

typedef HANDLE Thread;
typedef DWORD ThreadRet;

#else

#include <stdint.h>
#include <pthread.h>

typedef pthread_t Thread;
typedef void *ThreadRet;

#endif

typedef ThreadRet (*ThreadRoutine)(void *arg);

// Create a thread
bool threadCreate(Thread *thread, ThreadRoutine routine, void *arg);
// Wait until the thread exits
bool threadJoin(Thread thread, ThreadRet *status);
// Exit the current thread
NV_NORETURN void threadExit(ThreadRet status);

#endif // !NV_THREADS_H_
