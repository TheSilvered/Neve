#ifndef NV_RENDER_H_
#define NV_RENDER_H_

#include "nv_editor.h"

// Slice a string view using visual characters instead of units.
// Return a view into `str` that starts visually at `visualStart` and is
// at most `maxVisualLength` wide.
// If `maxVisualLength < 0` there is not max length.
// `outStartWidth` is the visual width of the characters before the slice.
// `outWidth` is the visual width of the returned slice.
// The slice will stop at any newline characters without including them.
StrView visualSlice(
    StrView *str,
    size_t visualStart,
    ptrdiff_t maxVisualLength,
    uint8_t tabStop,
    size_t *outStartWidth,
    size_t *outWidth
);

void renderFile(Editor *ed);
void renderStatusBar(Editor *ed);

#endif // !NV_RENDER_H_
