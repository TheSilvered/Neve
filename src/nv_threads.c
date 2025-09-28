#ifdef _WIN32
#include "win32/nv_threads_win32.c"
#else
#include "unix/nv_threads_unix.c"
#endif // !_WIN32
