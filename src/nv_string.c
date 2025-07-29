#include <assert.h>
#include <string.h>
#include "nv_mem.h"
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

    size_t newCap = (str->cap + reserve + 1) * 1.5;
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
    if (sv->len == 0) {
        return;
    }
    strReserve(str, sv->len);

    // StrView is not guaranteed to end with a NUL character, add it manually
    memcpy(str->buf + str->len, sv->buf, sv->len * sizeof(*sv->buf));
    str->len += sv->len;
    str->buf[str->len] = '\0';
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

char *strAsC(Str *str) {
    if (str->buf == NULL) {
        return "";
    } else {
        return (char *)str->buf;
    }
}

void strViewInitFromC(StrView *sv, const char *cStr) {
    sv->buf = (const UcdCh8 *)cStr;
    sv->len = strlen(cStr);
}

StrView strViewMakeFromC(const char *cStr) {
    StrView sv = {
        .buf = (const UcdCh8 *)cStr,
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
        idx += ucdCh8RunLen(sv->buf[idx]);
    }

    if (idx >= (ptrdiff_t)sv->len) {
        goto endReached;
    }
    size_t runLen = ucdCh8RunLen(sv->buf[idx]);

    if (runLen == 0 || idx + runLen > sv->len) {
        goto endReached;
    }

    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(sv->buf + idx);
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
    while (idx >= 0 && ucdCh8RunLen(sv->buf[idx]) == 0) {
        idx--;
    }
    if (idx < 0) {
        goto endReached;
    }
    size_t runLen = ucdCh8RunLen(sv->buf[idx]);
    if (runLen == 0 || idx + runLen > sv->len) {
        goto endReached;
    }
    if (outCP != NULL) {
        *outCP = ucdCh8ToCP(sv->buf + idx);
    }
    return idx;

endReached:
    if (outCP != NULL) {
        *outCP = -1;
    }
    return -1;
}

#ifdef _WIN32
#include "nv_unicode.h"

#define bufSize_ 4096

static char g_chBuf[bufSize_];
static wchar_t g_wchBuf[bufSize_];

const wchar_t *tempWStr(const char *str) {
    (void)ucdCh8StrToCh16Str(
        (const UcdCh8 *)str,
        strlen(str),
        (UcdCh16 *)g_wchBuf,
        bufSize_
    );
    return g_wchBuf;
}

const char *tempStr(const wchar_t *str) {
    (void)ucdCh16StrToCh8Str(
        (const UcdCh16 *)str,
        wcslen(str),
        (UcdCh8 *)g_chBuf,
        bufSize_
    );
    return g_chBuf;
}

#undef bufSize_

#endif // !_WIN32
