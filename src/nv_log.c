#include <stdarg.h>
#include "nv_log.h"

static File logFile = { 0 };

bool logInit(const char *filePath) {
    if (filePath == NULL) {
        filePath = "log.txt";
    }
    return fileOpen(&logFile, filePath, FileMode_Write) == FileIOResult_Success;
}

void logQuit(void) {
    fileClose(&logFile);
}

void logFmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)vfprintf(logFile.fp, fmt, args);
    va_end(args);
}
