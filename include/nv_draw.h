#ifndef NV_DRAW_H_
#define NV_DRAW_H_

#include "nv_tui.h"
#include "nv_screen.h"

void drawUI(Screen *screen, const UI *ui);
void drawBufPanel(Screen *screen, const UIBufPanel *panel);
void drawStatusBar(Screen *screen, const UIElement *statusBar);

#endif // !NV_DRAW_H_
