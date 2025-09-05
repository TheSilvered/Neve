#include <stdarg.h>
#include <stdlib.h>
#include "nv_log.h"
#include "nv_utils.h"

static File logFile = { 0 };

#ifdef _WIN32

bool logInit(const char *filePath) {
    if (filePath =! NULL) {
        return fileOpen(&logFile, filePath, FileMode_Write)
            == FileIOResult_Success;
    }

    UcdCh8 buf[1024];
    StrBuf sb = strBufMake(buf, NV_ARRLEN(buf));
    const char *appdata = getenv("LOCALAPPDATA");
    if (appdata == NULL) {
        return false;
    }

    strBufAppendC(&sb, appdata);
    if (sb.buf[sb.len - 1] != '\\' && sb.buf[sb.len - 1] != '/') {
        strBufAppendC(&sb, "\\");
    }
    strBufAppendC(&sb, "neve.log.txt");
    return fileOpen(&logFile, filePath, FileMode_Write) == FileIOResult_Success;
}

#else

bool logInit(const char *filePath) {
    if (filePath == NULL) {
        filePath = "~/.neve.log.txt";
    }
    return fileOpen(&logFile, filePath, FileMode_Write) == FileIOResult_Success;
}

#endif

void logQuit(void) {
    fileClose(&logFile);
}

void logFmt(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    (void)vfprintf(logFile.fp, fmt, args);
    va_end(args);
}
