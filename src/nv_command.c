#include <stdarg.h>
#include <stdio.h>
#include "nv_command.h"
#include "nv_editor.h"
#include "nv_mem.h"

#define _defaultBufSize 128

static CmdResult *_resultFmt(bool success, const char *fmt, va_list args) {
    CmdResult *result = memAllocBytes(sizeof(*result) + _defaultBufSize);
    result->success = success;
    result->msg.buf = (Utf8Ch *)(result + 1);
    int bufSize = vsnprintf(
        (char *)result->msg.buf,
        _defaultBufSize,
        fmt, args
    );
    if (bufSize < 0) {
        bufSize = 0;
    }
    if (bufSize < _defaultBufSize) {
        result->msg.len = (size_t)bufSize;
        return result;
    } else {
        result = memExpandBytes(result, sizeof(*result) + bufSize + 1);
        result->msg.buf = (Utf8Ch *)(result + 1);
        result->msg.len = vsnprintf(
            (char *)result->msg.buf,
            bufSize,
            fmt, args
        );
        return result;
    }
}

CmdResult *cmdResultFailed(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    CmdResult *result = _resultFmt(false, fmt, args);
    va_end(args);
    return result;
}

CmdResult *cmdResultSuccess(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    CmdResult *result = _resultFmt(true, fmt, args);
    va_end(args);
    return result;
}

CmdEntry *cmdEntryNew(const char *name, Cmd cmd) {
    size_t nameLen = strlen(name);
    CmdEntry *entry = memAllocBytes(sizeof(*entry) + nameLen + 1);
    entry->cmd = cmd;
    entry->nameLen = nameLen;
    memcpy(entry->name, name, nameLen + 1);
    return entry;
}

CmdResult *cmdQuit(StrView args) {
    if (args.len != 0) {
        return cmdResultFailed("unexpected arguments");
    }
    g_ed.running = false;
    return cmdResultSuccess("");
}
