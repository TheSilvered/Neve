#include <setjmp.h>

extern jmp_buf g_testJmpBuf;
extern int g_testAssertLine;
extern const char *g_testAssertFile;
extern const char *g_testAssertExpr;
extern const char *g_testAssertMsg;

#define nvAssertExpr(expr) do {                                                \
    if (!(expr)) {                                                             \
        g_testAssertExpr = #expr;                                              \
        g_testAssertFile = __FILE__;                                           \
        g_testAssertLine = __LINE__;                                           \
        longjmp(g_testJmpBuf, 1);                                              \
    }                                                                          \
    } while (0)

#define nvAssert(expr, msg) do {                                               \
    if (!(expr)) {                                                             \
        g_testAssertExpr = #expr;                                              \
        g_testAssertFile = __FILE__;                                           \
        g_testAssertLine = __LINE__;                                           \
        g_testAssertMsg = (msg);                                               \
        longjmp(g_testJmpBuf, 2);                                              \
    }                                                                          \
    } while (0)

