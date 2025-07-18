#include <stdlib.h>
#include <string.h>
#include "nv_string.h"

// NOTE: The actual capacity of the string buffer is one more than Str.cap
//       to allow for the NUL byte

Str *strNew(size_t reserve) {
    Str *str = malloc(sizeof(Str));
    if (str == NULL) {
        return NULL;
    }

    if (!strInit(str, reserve)) {
        free(str);
        return NULL;
    }
    return str;
}

Str *strNewFromC(const char *cStr) {
    Str *str = malloc(sizeof(Str));
    if (str == NULL) {
        return NULL;
    }

    if (!strInitFromC(str, cStr)) {
        free(str);
        return NULL;
    }
    return str;
}

bool strInit(Str *str, size_t reserve) {
    str->len = 0;
    str->cap = reserve + 1;
    if (reserve == 0) {
        str->buf = NULL;
        str->cap = 0;
    } else {
        // Keep the space for '\0'
        str->buf = calloc(reserve + 1, sizeof(*str->buf));
        if (str->buf == NULL) {
            str->cap = 0;
            return false;
        } else {
            str->cap = reserve;
            str->buf[0] = '\0';
        }
    }
    return true;
}

bool strInitFromC(Str *str, const char *cStr) {
    size_t len = strlen(cStr);
    str->len = len;
    str->cap = len;
    str->buf = malloc((len + 1) * sizeof(*cStr));
    if (str->buf == NULL) {
        str->len = 0;
        str->cap = 0;
        str->buf = NULL;
        return false;
    }
    // Copy the NUL character
    memcpy(str->buf, cStr, (len + 1) * sizeof(*cStr));
    return true;
}

void strFree(Str *str) {
    if (str == NULL) {
        return;
    }
    if (str->buf != NULL) {
        free(str->buf);
    }
    free(str);
}

void strDestroy(Str *str) {
    if (str == NULL) {
        return;
    }
    if (str->buf != NULL) {
        free(str->buf);
    }
    str->len = 0;
    str->cap = 0;
    str->buf = NULL;
}

bool strReserve(Str *str, size_t reserve) {
    if (reserve == 0 || str->len + reserve < str->cap) {
        return true;
    }

    size_t newCap = (str->cap + reserve) * 1.5;
    UcdCh8 *newBuf = NULL;
    if (str->buf == NULL) {
        newBuf = malloc((newCap + 1) * sizeof(*str->buf));
    } else {
        newBuf = realloc(str->buf, (newCap + 1) * sizeof(*str->buf));
    }
    if (newBuf == NULL) {
        return false;
    }
    str->cap = newCap;
    str->buf = newBuf;
    return true;
}

bool strAppendC(Str *str, const char *cStr) {
    size_t len = strlen(cStr);
    if (len == 0) {
        return true;
    }

    if (!strReserve(str, len)) {
        return false;
    }

    memcpy(str->buf + str->len, cStr, (len + 1) * sizeof(*cStr));
    str->len += len;
    return true;
}

bool strAppend(Str *str, const StrView *sv) {
    if (sv->len == 0) {
        return true;
    }

    if (!strReserve(str, sv->len)) {
        return false;
    }

    // StrView is not guaranteed to end with a NUL character, add it manually
    memcpy(str->buf + str->len, sv->buf, sv->len * sizeof(*sv->buf));
    str->len += sv->len;
    str->buf[str->len] = '\0';
    return true;
}

bool strClear(Str *str, size_t reserve) {
    if (reserve == 0) {
        strDestroy(str);
        return true;
    }

    UcdCh8 *newBuf = NULL;
    if (reserve == str->cap) {
        newBuf = str->buf;
    } else if (str->buf == NULL) {
        newBuf = malloc((reserve + 1) * sizeof(*str->buf));
    } else {
        newBuf = realloc(str->buf, (reserve + 1)* sizeof(*str->buf));
    }
    if (newBuf == NULL && str->cap > reserve) {
        newBuf = str->buf;
        reserve = str->cap;
    } else if (newBuf == NULL) {
        return false;
    }

    newBuf[0] = '\0';
    str->buf = newBuf;
    str->cap = reserve;
    str->len = 0;

    return true;
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

#ifdef _WIN32
#include "nv_unicode.h"

#define bufSize_ 512

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
