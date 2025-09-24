#ifdef _WIN32
#include "nv_threads_win32.c"
#else
#include "nv_threads_unix.c"
#endif // !_WIN32
