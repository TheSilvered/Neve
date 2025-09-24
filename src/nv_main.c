#include <assert.h>
#include <stdio.h>

#include "nv_editor.h"
#include "nv_error.h"
#include "nv_escapes.h"
#include "nv_file.h"
#include "nv_key_queue.h"
#include "nv_log.h"
#include "nv_term.h"
#include "nv_threads.h"

static KeyQueue g_keyQ = { 0 };
static Thread g_inputThread;
static bool g_inputThreadRun = true;

ThreadRet inputThreadRoutine(void *arg);

bool initNeve(void) {
    if (!termInit()) {
        errLog("failed to initialize the terminal");
        return false;
    }

    if (!threadCreate(&g_inputThread, inputThreadRoutine, NULL)) {
        errLog("failed to create thread");
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
        escDisableAltBuffer
        escCursorShow
        escCursorShapeDefault
    ));

    g_inputThreadRun = false;
    (void)threadJoin(g_inputThread, NULL);

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

ThreadRet inputThreadRoutine(void *arg) {
    (void)arg;
    while (g_inputThreadRun) {
        int32_t key = termGetKey();
        if (key == 0) {
            continue;
        }
        keyQueueEnq(&g_keyQ, key);
    }
    return 0;
}

// TODO: use wmain on Windows
int main(int argc, char **argv) {
    int ret = 0;
    if (!logInit(NULL)) {
        printf("Failed to generate log.\n");
        return 1;
    }

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
        int32_t key = keyQueueDeq(&g_keyQ);
        if (key < 0) {
            errLog("failed to read the key");
            goto exit;
        }
        editorHandleKey((uint32_t)key);
        editorRefresh();
    }

exit:
    quitNeve();
    logQuit();
    return ret;
}
