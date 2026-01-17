#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "clib_mem.h"
#include "nv_string.h"

// NOTE: The actual capacity of the string buffer is one more than Str.cap
//       to allow for the NUL byte

Str *strNew(size_t reserve) {
    Str *str = memAlloc(1, sizeof(Str));
    strInit(str, reserve);
    return str;
}

Str *strNewFromC(const char *cStr) {
    Str *str = memAlloc(1, sizeof(Str));
    strInitFromC(str, cStr);
    return str;
}

void strInit(Str *str, size_t reserve) {
    str->len = 0;
    str->cap = reserve + 1;
    if (reserve == 0) {
        str->buf = NULL;
        str->cap = 0;
    } else {
        // Keep the space for '\0'
        str->buf = memAlloc(reserve + 1, sizeof(*str->buf));
        str->cap = reserve;
        str->buf[0] = '\0';
    }
}

void strInitFromC(Str *str, const char *cStr) {
    size_t len = strlen(cStr);
    str->len = len;
    str->cap = len;
    str->buf = memAlloc(len + 1, sizeof(*cStr));
    // Copy the NUL character
    memcpy(str->buf, cStr, (len + 1) * sizeof(*cStr));
}

void strFree(Str *str) {
    if (str == NULL) {
        return;
    }
    memFree(str->buf);
    memFree(str);
}

void strDestroy(Str *str) {
    if (str == NULL) {
        return;
    }
    memFree(str->buf);
    str->buf = NULL;
    str->len = 0;
    str->cap = 0;
}

void strReserve(Str *str, size_t reserve) {
    if (reserve == 0 || str->len + reserve < str->cap) {
        return;
    }

    size_t newCap = (size_t)((str->cap + reserve + 1) * 1.5);
    str->buf = memChange(str->buf, newCap, sizeof(*str->buf));
    str->cap = newCap;
}

void strAppendC(Str *str, const char *cStr) {
    size_t len = strlen(cStr);
    if (len == 0) {
        return;
    }
    strReserve(str, len);

    memcpy(str->buf + str->len, cStr, (len + 1) * sizeof(*cStr));
    str->len += len;
}

void strAppend(Str *str, const StrView *sv) {
    strAppendRaw(str, sv->buf, sv->len);
}

nvUnixFmt(2, 3) void strAppendFmt(Str *str, nvWinFmt const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);

    size_t len = vsnprintf(buf, nvArrlen(buf), fmt, args);
    strAppendRaw(str, (const Utf8Ch *)buf, len);
}

void strAppendRaw(Str *str, const Utf8Ch *buf, size_t len) {
    if (len == 0) {
        return;
    }
    strReserve(str, len);

    memcpy(str->buf + str->len, buf, len * sizeof(*buf));
    str->len += len;
    str->buf[str->len] = '\0';
}

void strRepeat(Str *str, char ch, size_t count) {
    if (count == 0 || (uint8_t)ch > 0x7f) {
        return;
    }

    strReserve(str, count);
    memset(str->buf + str->len, ch, count);
    str->len += count;
    str->buf[str->len] = '\0';
}

void strPop(Str *str, size_t count) {
    if (count == 0) {
        return;
    }

    ptrdiff_t i = str->len;
    for (
        i = strViewPrev((StrView *)str, i, NULL);
        i > 0;
        i = strViewPrev((StrView *)str, i, NULL)
    ) {
        if (--count == 0) {
            break;
        }
    }
    // This should never happen, just in case
    if (i < 0) {
        i = 0;
    }

    str->len = i;
    str->buf[str->len] = '\0';

    // Shrink the buffer by half if the string is less than a quarter full
    if (str->len < str->cap / 4) {
        str->buf = memShrink(str->buf, str->len * 2 + 1, sizeof(*str->buf));
        str->cap = str->len * 2;
    }
}

void strCut(Str *str, size_t len) {
    if (len >= str->len) {
        return;
    }
    str->len = len;
    str->buf[str->len] = '\0';
    if (str->len < str->cap / 4) {
        str->buf = memShrink(str->buf, str->len * 2 + 1, sizeof(*str->buf));
        str->cap = str->len * 2;
    }
}

void strClear(Str *str, size_t reserve) {
    if (reserve == 0) {
        strDestroy(str);
        return;
    }

    str->buf = memChange(str->buf, reserve + 1, sizeof(*str->buf));
    str->cap = reserve;
    str->len = 0;
    str->buf[0] = '\0';
}

char *strAsC(const Str *str) {
    if (str->buf == NULL) {
        return "";
    } else {
        return (char *)str->buf;
    }
}

void strViewInitFromC(StrView *sv, const char *cStr) {
    sv->buf = (const Utf8Ch *)cStr;
    sv->len = strlen(cStr);
}

StrView strViewMakeFromC(const char *cStr) {
    StrView sv = {
        .buf = (const Utf8Ch *)cStr,
        .len = strlen(cStr)
    };
    return sv;
}

void strBufInit(StrBuf *sb, char *buf, size_t bufSize) {
    sb->buf = buf;
    sb->len = 0;
    sb->bufSize = bufSize;
    if (bufSize != 0) {
        buf[0] = '\0';
    }
}

StrBuf strBufMake(char *buf, size_t bufSize) {
    StrBuf sb = {
        .buf = buf,
        .len = 0,
        .bufSize = bufSize
    };
    if (bufSize != 0) {
        buf[0] = '\0';
    }
    return sb;
}

bool strBufAppendC(StrBuf *sb, const char *cStr) {
    size_t len = strlen(cStr);
    if (sb->len + len + 1 >= sb->bufSize) {
        return false;
    }
    memcpy(sb->buf + sb->len, cStr, (len + 1) * sizeof(*cStr));
    sb->len += len;
    return true;
}

bool strBufAppend(StrBuf *sb, const StrView *sv) {
    if (sb->len + sv->len + 1 >= sb->bufSize) {
        return false;
    }
    memcpy(sb->buf + sb->len, sv->buf, sv->len * sizeof(*sv->buf));
    sb->len += sv->len;
    sb->buf[sb->len] = '\0';
    return true;
}

void strBufPop(StrBuf *sb, size_t count) {
    if (count == 0) {
        return;
    }

    ptrdiff_t i = sb->len;
    for (
        i = strViewPrev((StrView *)sb, i, NULL);
        i > 0;
        i = strViewPrev((StrView *)sb, i, NULL)
    ) {
        if (--count == 0) {
            break;
        }
    }
    // This should never happen, just in case
    if (i < 0) {
        i = 0;
    }

    sb->len = i;
    sb->buf[i] = '\0';
}

void strBufClear(StrBuf *sb) {
    sb->buf[0] = '\0';
    sb->len = 0;
}

ptrdiff_t strViewNext(const StrView *sv, ptrdiff_t idx, UcdCP *outCP) {
    if (idx >= (ptrdiff_t)sv->len) {
        goto endReached;
    } else if (idx < 0) {
        idx = 0;
    } else {
        idx += utf8ChRunLen(sv->buf[idx]);
    }

    if (idx >= (ptrdiff_t)sv->len) {
        goto endReached;
    }
    uint8_t runLen = utf8ChRunLen(sv->buf[idx]);

    if (runLen == 0 || (size_t)idx + runLen > sv->len) {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = utf8ChToCP(sv->buf + idx);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

ptrdiff_t strViewPrev(const StrView *sv, ptrdiff_t idx, UcdCP *outCP) {
    if (idx == 0) {
        goto endReached;
    } else if (idx < 0) {
        idx = sv->len;
    }
    idx--;
    while (idx >= 0 && utf8ChRunLen(sv->buf[idx]) == 0) {
        idx--;
    }
    if (idx < 0) {
        goto endReached;
    }
    uint8_t runLen = utf8ChRunLen(sv->buf[idx]);
    if (runLen == 0 || (size_t)idx + runLen > sv->len) {
        goto endReached;
    }
    if (outCP != NULL) {
        *outCP = utf8ChToCP(sv->buf + idx);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

#ifdef _WIN32
#include "unicode/nv_utf.h"

static char g_chBuf[4096];
static wchar_t g_wchBuf[4096];

const wchar_t *tempWStr(const char *str) {
    (void)utf8ToUtf16(
        (const Utf8Ch *)str,
        strlen(str),
        (Utf16Ch *)g_wchBuf,
        nvArrlen(g_wchBuf)
    );
    return g_wchBuf;
}

const char *tempStr(const wchar_t *str) {
    (void)utf16ToUtf8(
        (const Utf16Ch *)str,
        wcslen(str),
        (Utf8Ch *)g_chBuf,
        nvArrlen(g_chBuf)
    );
    return g_chBuf;
}

#endif // !_WIN32
