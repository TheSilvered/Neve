#include <stdarg.h>
#include <stdio.h>
#include "nv_command.h"
#include "nv_editor.h"
#include "clib_mem.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

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
    if (editorTryExit()) {
        return cmdResultSuccess("");
    } else {
        return cmdResultFailed("unsaved changes");
    }
}

CmdResult *cmdForceQuit(StrView args) {
    if (args.len != 0) {
        return cmdResultFailed("unexpected arguments");
    }
    editorForceExit();
    return cmdResultSuccess("");
}

CmdResult *cmdSaveAndQuit(StrView args) {
    CmdResult *saveResult = cmdSave(args);
    if (!saveResult->success) {
        return saveResult;
    }
    return cmdQuit(args);
}

CmdResult *cmdWorkingDir(StrView args) {
    if (args.len != 0) {
        return cmdResultFailed("unexpected arguments");
    }

#define _bufLen 1024

#ifdef _WIN32
    wchar_t wbuf[_bufLen];
    wchar_t *dir = _wgetcwd(wbuf, _bufLen);
#else
    char buf[_bufLen];
    char *dir = getcwd(buf, _bufLen);
#endif
    if (dir == NULL) {
        return cmdResultFailed("failed to get the current working directory");
    }
#ifdef _WIN32
    const char *buf = tempStr(dir);
#endif
    return cmdResultSuccess("%s", buf);

#undef _bufLen
}

CmdResult *cmdSave(StrView args) {
    if (args.len != 0) {
        return cmdResultFailed("unexpected arguments");
    }
    Buf *buf = bufRef(&g_ed.buffers, g_ed.ui.bufPanel.bufHd);
    if (buf == NULL) {
        return cmdResultFailed("no open buffer");
    }
    FileIOResult result = bufWriteToDisk(buf);
    switch (result) {
    case FileIOResult_Success:
        return cmdResultSuccess("saved at "strFmt, strArg(&buf->path));
    case FileIOResult_BadPath:
        return cmdResultFailed("invalid path '"strFmt"'", strArg(&buf->path));
    case FileIOResult_PermissionDenied:
        return cmdResultFailed("write permission denied");
    default:
        return cmdResultFailed("failed to write");
    }
}

CmdResult *cmdSaveAs(StrView args) {
    if (args.len == 0) {
        return cmdResultFailed("no path given");
    }
    if (!bufSetPath(&g_ed.buffers, g_ed.ui.bufPanel.bufHd, &args)) {
        return cmdResultFailed("no open buffer");
    }

    return cmdSave((StrView){ 0 });
}
