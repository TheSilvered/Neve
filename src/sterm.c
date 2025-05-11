#include "sterm.h"

#ifdef _MSC_VER
#include "sterm_win32.c"
#else
#include "sterm_unix.c"
#endif
