#include <assert.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_error.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_log.h"
#include "nv_term.h"

bool initNeve(void) {
    if (!termInit()) {
        errLog("failed to initialize the terminal");
        return false;
    }

    if (!termEnableRawMode(1)) {
        errLog("failed to enable raw mode");
        return false;
    }

    editorInit();
    return true;
}

void quitNeve(void) {
    termWrite(sLen(
        escScreenClear
        escCursorShow
        escCursorShapeDefault
    ));

    editorQuit();
    termQuit();
}

void loadOrCreateFile(const char *path) {
    File file;

    switch (fileOpen(&file, path, FileMode_Read)) {
    case FileIOResult_FileNotFound: {
        StrView pathSv = strViewMakeFromC(path);
        bufSetPath(&g_ed.fileBuf, &pathSv);
        return;
    }
    case FileIOResult_Success:
        bufDestroy(&g_ed.fileBuf);
        bufInitFromFile(&g_ed.fileBuf, &file);
        fileClose(&file);
        return;
    default:
        return;
    }
}

// TODO: use wmain on Windows
int main(int argc, char **argv) {
    logInit(NULL);

    if (argc > 2) {
        printf("Usage: neve [file]\n");
        return 1;
    }

    if (!initNeve()) {
        return 1;
    }

    if (argc == 2) {
        loadOrCreateFile(argv[1]);
    }

    while (g_ed.running) {
        int32_t key = termGetKey();
        if (key < 0) {
            errLog("failed to read the key");
            return 1;
        }
        editorHandleKey((uint32_t)key);
        editorRefresh();
    }

    quitNeve();
    logQuit();
    return 0;
}
