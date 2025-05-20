#include <stdlib.h>
#include <string.h>
#include "nv_str.h"

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
    } else {
        // Always keep the space for '\0'
        str->buf = calloc(reserve + 1, sizeof(*str->buf));
        if (str->buf == NULL) {
            str->cap = 0;
            return false;
        } else {
            str->cap = reserve + 1;
            str->buf[0] = '\0';
        }
    }
    return true;
}

bool strInitFromC(Str *str, const char *cStr) {
    size_t len = strlen(cStr);
    str->len = len;
    str->cap = len + 1;
    str->buf = malloc((len + 1) * sizeof(*cStr));
    if (str->buf == NULL) {
        str->len = 0;
        str->cap = 0;
        str->buf = NULL;
        return false;
    }
    // Also copy the NUL character
    memcpy(str->buf, cStr, (len + 1) * sizeof(*cStr));
    return true;
}

StrView strViewMakeC(const char *cStr) {
    StrView sv = {
        .buf = (const UcdCh8 *)cStr,
        .len = strlen(cStr)
    };
    return sv;
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
    str->buf = 0;
}

bool strReserve(Str *str, size_t reserve) {
    if (reserve == 0 || str->len + reserve + 1 < str->cap) {
        return true;
    }

    size_t newCap = (str->cap + reserve + 1) * 1.5;
    UcdCh8 *newBuf = NULL;
    if (str->buf == NULL) {
        newBuf = malloc(newCap * sizeof(*str->buf));
    } else {
        newBuf = realloc(str->buf, newCap * sizeof(*str->buf));
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
    if (!strReserve(str, len)) {
        return false;
    }

    memcpy(str->buf + str->len, cStr, (len + 1) * sizeof(*cStr));
    str->len += len;
    return true;
}

bool strAppend(Str *str, StrView *sv) {
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
    if (str->buf == NULL) {
        newBuf = malloc((reserve + 1) * sizeof(*str->buf));
    } else {
        newBuf = realloc(str->buf, (reserve + 1)* sizeof(*str->buf));
    }
    if (newBuf == NULL) {
        return false;
    }

    newBuf[0] = '\0';
    str->buf = newBuf;
    str->cap = reserve + 1;
    str->len = 0;

    return true;
}

