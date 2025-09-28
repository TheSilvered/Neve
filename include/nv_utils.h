#ifndef NV_UTILS_H_
#define NV_UTILS_H_

#ifdef _MSC_VER
#define NV_WIN_FMT _Printf_format_string_
#else
#define NV_WIN_FMT
#endif // !NV_WIN_FMT

#ifdef __GNUC__
#define NV_UNIX_FMT(str, args) __attribute__((format(printf, str, args)))
#else
#define NV_UNIX_FMT(str, args)
#endif // !NV_UNIX_FMT

#ifdef _MSC_VER
#define NV_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define NV_NORETURN __attribute__((noreturn))
#else
#define NV_NORETURN
#endif // !NV_NORETURN

#ifdef _MSC_VER
#define NV_UNREACHABLE __assume(0)
#elif defined(__GNUC__)
#define NV_UNREACHABLE __builtin_unreachable()
#else
#define NV_UNREACHABLE
#endif // !NV_UNREACHABLE

#define NV_ARRLEN(arr) (sizeof(arr) / sizeof(*(arr)))
#define NV_MIN(a, b) ((a) < (b) ? (a) : (b))
#define NV_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif // !NV_UTILS_H_
