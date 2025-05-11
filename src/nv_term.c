#include "nv_term.h"

#ifdef _MSC_VER
#include "nv_term_win32.c"
#else
#include "nv_term_unix.c"
#endif
