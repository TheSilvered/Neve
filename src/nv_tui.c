#include "nv_tui.h"

void uiPanelInit(UIPanel *panel, UIPanelKind kind, UIKeyHandler kh) {
    panel->x = 0;
    panel->y = 0;
    panel->w = 0;
    panel->h = 0;
    panel->kind = kind;
    panel->_keyHandler = kh;
}

UIPanel *uiPanelBufNew(Buf *buf) {
    UIPanelBuf *bufPanel = memAlloc(1, sizeof(UIPanelBuf));

    uiPanelInit(&bufPanel->panel, UIPanelKind_Buf, NULL);
    bufPanel->buf = buf;

    return &bufPanel->panel;
}

