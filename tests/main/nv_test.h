#ifndef NV_TEST_H_
#define NV_TEST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "nv_mem.h"
#include "nv_utils.h" // Useful inside the tests

// Length of a char array
#define chArrLen(s) (sizeof(s)/sizeof(*(s)) - 1)

// TODO: testCapture

typedef struct Test {
    const char *name;
    void (*callback)(void);
} Test;

// Initialize a test using `func` as the callback and the name
#define testMake(func) { .callback = (func), .name = #func }

// Assert that `expr` is true
#define testAssert(expr) _testAssert((expr), #expr, __FILE__, __LINE__)
// Assert that `expr` is true
// Usage: testAssertWith(var == 1) { /* tests to evaluate if var is 1 */ }
#define testAssertWith(expr) \
    if (_testAssertWith((expr), #expr, __FILE__, __LINE__))
// Assert that `expr` is true, otherwise exit from the test
#define testAssertRequire(expr) do { \
    if (!_testAssertRequire((expr), #expr, __FILE__, __LINE__)) return; \
    } while (0)

void _testAssert(bool expr, const char *exprStr, const char *file, int line);
bool _testAssertWith(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
);
bool _testAssertRequire(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
);
void _testCheckAllocs(void);
bool _testFailed(void);

// Put the initializers of the tests defined in the file
// Usage: testList(testMake(myTestFunc), { myFunc, "A name" })
#define testList(...)                                                          \
    int main(void) {                                                           \
        memInit();                                                             \
        Test tests[] = { __VA_ARGS__ };                                        \
        size_t testCount = sizeof(tests) / sizeof(*tests);                     \
        for (size_t i = 0; i < testCount; i++) {                               \
            printf("running test %s...\n", tests[i].name);                     \
            fflush(stdout);                                                    \
            tests[i].callback();                                               \
            _testCheckAllocs();                                                \
        }                                                                      \
        memQuit();                                                             \
        return _testFailed() ? 1 : 0;                                          \
    }

#endif // !NV_TEST_H_
