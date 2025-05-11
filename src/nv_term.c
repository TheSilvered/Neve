#include "nv_term.h"

#ifdef _WIN32
#include "nv_term_win32.c"
#else
#include "nv_term_unix.c"
#endif
