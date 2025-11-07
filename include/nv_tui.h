#ifndef NV_TUI_H_
#define NV_TUI_H_

#include <stdint.h>
#include "nv_term.h"

/*
--------------------------------------------------------------------------------
                 |[tab1         x][tab2         x][tab3         x]
                 |
                 |
                 |
                 |
                 |
                 |
                 |
                 |
                 |        /-------Popup Window-------\
Side bar         |        |                          |
                 |        |                          |
                 |        |      [Yes] [No] [Cancel] |
                 |        \--------------------------/
                 |
                 |
                 |
                 |
                 |
                 |--------------------------------------------------------------
                 |                           Info box
                 |
################################## Status Bar ##################################
--------------------------------------------------------------------------------
*/

typedef enum UIEventResult {
    UIEventResult_Ignored,
    UIEventResult_Handled,
    UIEventResult_Consumed,
    UIEventResult_Error
} UIEventResult;

typedef enum UIEventKind {
    UIEventKind_KeyPress,
    UIEventKind_Resize
} UIEventKind;

typedef struct UIEventKeyPress {
    TermKey key;
    UcdCP cp;
    bool ctrl, alt;
} UIEventKeyPress;

typedef struct UIEvent {
    UIEventKind kind;
    union {
        UIEventKeyPress keyPress;
    } val;
} UIEvent;

typedef UIEventResult (*UIEventHandler)(UIEvent *event);

typedef struct UIElement {
    int16_t x, y;
    uint16_t w, h;
} UIElement;

#endif // !NV_TUI_H_
