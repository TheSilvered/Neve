#ifndef NV_MEM_OPTIONS_H_
#define NV_MEM_OPTIONS_H_

#include <stdlib.h>
#include "nv_utils.h"

#define CLIB_MEM_ABORT_ON_FAIL
#ifdef NV_DEBUG
#define CLIB_MEM_TRACE_ALLOCS
#endif // !NV_DEBUG

#define memAssert(expr) do {                                                   \
    nvAssertExpr(expr);                                                        \
    if (!(expr)) abort();                                                      \
    } while (0)

#endif // !NV_MEM_OPTIONS_H_
