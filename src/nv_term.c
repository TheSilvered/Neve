#include "nv_term.h"

#ifdef _WIN32
#include "nv_term_win32.c"
#else
#include "nv_term_unix.c"
#endif // !_WIN32

bool termClearScreen(void) {
    return termWrite(TERM_ESC "2J", 4)
        && termWrite(TERM_ESC "3J", 4)
        && termWrite(TERM_ESC "H", 3);
}
