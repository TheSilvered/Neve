#ifndef NV_TUI_H_
#define NV_TUI_H_

#include <stdint.h>
#include "nv_buffer.h"
#include "nv_term.h"

/*
|--------------------------------------------------------------------------------|
|                 |[tab1         x][tab2         x][tab3         x]              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |        /-------Popup Window-------\                          |
|    Side bar     |        |                          |                          |
|                 |        |                          |                          |
|                 |        |      [Yes] [No] [Cancel] |                          |
|                 |        \--------------------------/                          |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |                                                              |
|                 |--------------------------------------------------------------|
|                 |                           Info box                           |
|                 |                                                              |
|################################## Status Bar ##################################|
|--------------------------------------------------------------------------------|
*/

typedef struct UIBufPanel UIBufPanel;

typedef bool (*UIKeyHandler)(UIBufPanel *panel, int32_t key);
typedef void (*UIUpdater)(UIBufPanel *panel);

typedef enum UIBufMode {
    UIBufMode_Normal,
    UIBufMode_Edit,
    UIBufMode_Selection
} UIBufMode;

struct UIBufPanel {
    Buf *buf;
    UIBufMode mode;
    uint16_t w, h;
    size_t scrollX, scrollY;
    UIKeyHandler keyHandler;
    UIUpdater updater;
};

// TODO: separate buffers and tabs
void uiBufPanelInit(UIBufPanel *panel, Buf *buf);

#endif // !NV_TUI_H_
