#ifdef _WIN32
#include "win32/nv_time_win32.c"
#else
#include "unix/nv_time_unix.c"
#endif // !_WIN32
