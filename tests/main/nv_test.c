#include <stdlib.h>
#include <string.h>
#include "nv_test.h"

#include "nv_mem.h"

/* ----------------------------- Test Functions ----------------------------- */

static bool g_failed = false;

void testAssert_(bool expr, const char *exprStr, const char *file, int line) {
    if (expr) {
        return;
    }

    g_failed = true;
    printf("  %s:%d %s\n", file, line, exprStr);
    fflush(stdout);
}

bool testAssertWith_(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
) {
    if (expr) {
        return true;
    }

    g_failed = true;
    printf("  %s:%d %s\n    Some checks were skipped.\n", file, line, exprStr);
    fflush(stdout);
    return false;
}

bool testAssertRequire_(
    bool expr,
    const char *exprStr,
    const char *file,
    int line
) {
    if (expr) {
        return true;
    }

    g_failed = true;
    printf(
        "  %s:%d %s\n"
        "    Assertion is required to continue the test.\n",
        file, line, exprStr
    );
    fflush(stdout);
    return false;
}

/* ----------------------------- Program Entry ------------------------------ */

int main(void) {
    size_t testCount;
    Test *tests = testGetTests_(&testCount);

    for (size_t i = 0; i < testCount; i++) {
        printf("running test %s...\n", tests[i].name);
        fflush(stdout);
        tests[i].callback();
    }

#ifndef NDEBUG
    if (memHasAllocs()) {
        memPrintAllocs();
        return g_failed ? 2 : 1;
    }
#endif // !NDEBUG

    return g_failed ? 1 : 0;
}
