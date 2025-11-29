#ifndef NV_ARRAY_H_
#define NV_ARRAY_H_

#include <string.h>
#include <assert.h>
#include "nv_mem.h"

// Dynamic array macros.
// Define a dynamic array as a struct with the fields `items`, `len` and `cap`.
// Items is required to be a pointer.

#define Arr(T) struct { T *items; size_t len, cap; }

#define arrReserve(arr, count)                                                 \
    do {                                                                       \
        if ((arr)->len + (count) > (arr)->cap) {                               \
            (arr)->cap = (size_t)(((arr)->len + (count)) * 1.5);               \
            (arr)->items = memExpand(                                          \
                (arr)->items,                                                  \
                (arr)->cap,                                                    \
                sizeof(*(arr)->items)                                          \
            );                                                                 \
        }                                                                      \
    } while (0)

#define arrResize(arr, requiredLen)                                            \
    do {                                                                       \
        if ((requiredLen) < (arr)->len) {                                      \
            (arr)->len = (requiredLen);                                        \
        }                                                                      \
        if ((requiredLen) == (arr)->len) {                                     \
            break;                                                             \
        } else if ((requiredLen) == 0) {                                       \
            memFree((arr)->items);                                             \
            (arr)->items = NULL;                                               \
            (arr)->len = 0;                                                    \
            (arr)->cap = 0;                                                    \
        } else if ((requiredLen) < (arr)->cap / 4) {                           \
            (arr)->items = memShrink(                                          \
                (arr)->items,                                                  \
                (arr)->cap / 2,                                                \
                sizeof(*(arr)->items)                                          \
            );                                                                 \
            (arr)->cap /= 2;                                                   \
        } else if ((requiredLen) > (arr)->cap) {                               \
            size_t newCap = (requiredLen) + (requiredLen) / 2;                 \
            (arr)->items = memChange(                                          \
                (arr)->items,                                                  \
                newCap,                                                        \
                sizeof(*(arr)->items)                                          \
            );                                                                 \
            (arr)->cap = newCap;                                               \
        }                                                                      \
    } while (0)

#define arrAppend(arr, ...)                                                    \
    do {                                                                       \
        arrReserve((arr), 1);                                                  \
        (arr)->items[(arr)->len++] = (__VA_ARGS__);                            \
    } while (0)

#define arrInsert(arr, idx, ...)                                               \
    do {                                                                       \
        assert((idx) < (arr)->len);                                            \
        arrReserve((arr), 1);                                                  \
        memmove(                                                               \
            &(arr)->items[(idx) + 1],                                          \
            &(arr)->items[(idx)],                                              \
            sizeof(*(arr)->items) * ((arr)->len - (idx))                       \
        );                                                                     \
        (arr)->items[(idx)] = (__VA_ARGS__);                                   \
        (arr)->len++;                                                          \
    } while (0)

#define arrRemove(arr, idx)                                                    \
    do {                                                                       \
        assert((idx) < (arr)->len);                                            \
        memmove(                                                               \
            &(arr)->items[(idx)],                                              \
            &(arr)->items[(idx) + 1],                                          \
            sizeof(*(arr)->items) * ((arr)->len - (idx) - 1)                   \
        );                                                                     \
        (arr)->len--;                                                          \
        if ((arr)->len < ((arr)->cap >> 2)) {                                  \
            arrResize((arr), (arr)->len * 2);                                  \
        }                                                                      \
    } while (0)

#define arrClear(arr)                                                        \
    do {                                                                       \
        memFree((arr)->items);                                                 \
        (arr)->items = NULL;                                                   \
        (arr)->len = 0;                                                        \
        (arr)->cap = 0;                                                        \
    } while (0)

#endif // !NV_ARRAY_H_
