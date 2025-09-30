#ifndef NV_LOGGING_H_
#define NV_LOGGING_H_

#include <stdbool.h>
#include "nv_utils.h"

bool logInit(const char *filePath);
void logQuit(void);

NV_UNIX_FMT(1, 2) void logFmt(NV_WIN_FMT const char *fmt, ...);

#endif // !NV_LOGGING_H_
