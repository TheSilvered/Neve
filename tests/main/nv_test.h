#ifndef NV_TEST_H_
#define NV_TEST_H_

#include <stdbool.h>
#include <stddef.h>

// TODO: testCapture

typedef struct Test {
    const char *name;
    void (*callback)(void);
} Test;

// Assert that `expr` is true
#define testAssert(expr) testAssert_((expr), #expr, __FILE__, __LINE__)
// Assert that `expr` is true
// Usage: testAssertWith(var == 1) { /* tests to evaluate if var is 1 */ }
#define testAssertWith(expr) \
    if (testAssertWith_((expr), #expr, __FILE__, __LINE__))
// Assert that `expr` is true, otherwise exit from the test
#define testAssertRequire(expr) do { \
    if (testAssertRequire_((expr), #expr, __FILE__, __LINE__)) return 1; \
    } while (0)

// Helper for `testAssert`
void testAssert_(bool expr, const char *exprStr, const char *file, int line);
// Helper for `testAssertWith`
bool testAssertWith_(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
);
// Helper for `testAssertRequire`
bool testAssertRequire_(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
);

Test *fileTests(size_t *outTestCount);

#endif // !NV_TEST_H_
