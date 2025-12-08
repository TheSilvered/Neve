#ifndef NV_LOGGING_H_
#define NV_LOGGING_H_

#include <stdbool.h>
#include "nv_utils.h"

bool logInit(const char *filePath);
void logQuit(void);

nvUnixFmt(1, 2) void logFmt(nvWinFmt const char *fmt, ...);

#endif // !NV_LOGGING_H_
