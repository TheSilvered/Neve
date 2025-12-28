#ifndef NV_COMMAND_H_
#define NV_COMMAND_H_

#include "nv_array.h"
#include "nv_string.h"

typedef struct CmdResult {
    bool success;
    StrView msg;
} CmdResult;

typedef CmdResult *(*Cmd)(StrView);

typedef struct CmdEntry {
    Cmd cmd;
    size_t nameLen;
    char name[];
} CmdEntry;

typedef Arr(CmdEntry *) CmdMap;

CmdEntry *cmdEntryNew(const char *name, Cmd cmd);
CmdResult *cmdResultSuccess(const char *fmt, ...);
CmdResult *cmdResultFailed(const char *fmt, ...);

CmdResult *cmdQuit(StrView args);

#endif // !NV_COMMAND_H_
