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

typedef struct UIElement UIElement;

typedef bool (*UIKeyHandler)(UIElement *panel, int32_t key);
typedef void (*UIUpdater)(UIElement *panel);

struct UIElement {
    int16_t x, y;
    uint16_t w, h;
    UIKeyHandler keyHandler;
    UIUpdater updater;
};

typedef enum UIBufMode {
    UIBufMode_Normal,
    UIBufMode_Edit,
    UIBufMode_Selection
} UIBufMode;

typedef struct UIBufPanel {
    UIElement elem;
    BufHandle bufHd;
    UIBufMode mode;
    size_t scrollX, scrollY;
} UIBufPanel;

typedef enum UICmdInputState {
    UICmdInput_Confirmed,
    UICmdInput_Canceled,
    UICmdInput_Inserting
} UICmdInputState;

typedef struct UICmdInput {
    UIElement elem;
    Ctx ctx;
    size_t scroll;
    UICmdInputState state;
} UICmdInput;

typedef struct UI {
    UIElement elem;
    UIBufPanel bufPanel;
    UIElement statusBar;
    UICmdInput cmdInput;
} UI;

void uiInit(UI *ui);
void uiResize(UI *ui, uint16_t w, uint16_t h);

void uiUpdate(UIElement *elem);
bool uiHandleKey(UIElement *elem, int32_t key);

void uiBufPanelInit(UIBufPanel *panel);
void uiCmdInputInit(UICmdInput *cmdInput);

#endif // !NV_TUI_H_
