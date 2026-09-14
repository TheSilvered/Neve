#ifndef NV_UTILS_H_
#define NV_UTILS_H_

#ifdef USR_UTILS_HEADER
#include USR_UTILS_HEADER
#endif // !USR_UTILS_HEADER

#ifdef _MSC_VER
#define nvWinFmt _Printf_format_string_
#else
#define nvWinFmt
#endif // !nvWinFmt

#ifdef __GNUC__
#define nvUnixFmt(str, args) __attribute__((format(printf, str, args)))
#else
#define nvUnixFmt(str, args)
#endif // !nvUnixFmt

#ifdef _MSC_VER
#define NvNoreturn __declspec(noreturn)
#elif defined(__GNUC__)
#define NvNoreturn __attribute__((noreturn))
#else
#define NvNoreturn
#endif // !NvNoreturn

#ifdef _MSC_VER
#define nvUnreachable (__assume(0))
#elif defined(__GNUC__)
#define nvUnreachable __builtin_unreachable()
#else
#define nvUnreachable
#endif // !nvUnreachable

#ifndef nvAssertExpr
#include <assert.h>
#define nvAssertExpr(expr) assert(expr)
#endif // !nvAssertExpr

#ifndef nvAssert
#define nvAssert(expr, msg) nvAssertExpr(expr && msg)
#endif // !nvAssertExpr

#define nvArrlen(arr) (sizeof(arr) / sizeof(*(arr)))
#define nvMin(a, b) ((a) < (b) ? (a) : (b))
#define nvMax(a, b) ((a) > (b) ? (a) : (b))

#endif // !NV_UTILS_H_
