#ifndef NV_LOG_H_
#define NV_LOG_H_

#include "nv_file.h"

bool logInit(const char *filePath);
void logQuit(void);

void logFmt(const char *fmt, ...);

#endif // !NV_LOG_H_
