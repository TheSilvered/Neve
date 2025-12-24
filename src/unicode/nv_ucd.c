#include <assert.h>
#include "unicode/nv_ucd.h"
#include "nv_utils.h"

#include "nv_ucd_tables.c"
#define _blockIdxMask ((1<<_ucdShift) - 1)

UcdCPInfo ucdGetCPInfo(UcdCP cp) {
    assert(cp <= ucdCPMax);
    assert(cp >= 0);
    if (cp < 0 || cp > ucdCPMax) {
        cp = 0;
    }
    uint8_t blockIdx = g_blockIndices[cp >> _ucdShift];
    uint8_t cpInfoIdx = g_infoBlocks[
        (blockIdx << _ucdShift) + (cp & _blockIdxMask)
    ];

    return g_cpInfo[cpInfoIdx];
}

bool ucdIsCPValid(UcdCP cp) {
    return cp <= ucdCPMax
        && cp >= 0
        && (cp < ucdHighSurrogateFirst || cp > ucdLowSurrogateLast);
}

bool ucdIsCPNoncharacter(UcdCP cp) {
    return (cp & 0xFFFF) == 0xFFFE
        || (cp & 0xFFFF) == 0xFFFF
        || (cp >= 0xFDD0 && cp <= 0xFDEF);
}

uint8_t ucdCPWidth(UcdCP cp, uint8_t tabStop, size_t currWidth) {
    if (cp == '\t') {
        return tabStop - (currWidth % tabStop);
    }

    // Short path for ASCII printable characters
    if (cp >= ' ' && cp <= '~') {
        return 1;
    } else {
        UcdCPInfo info = ucdGetCPInfo(cp);
        switch (info.width) {
        case UcdWidth_Fullwidth:
        case UcdWidth_Wide:
            return 2;
        case UcdWidth_Ambiguous:
        case UcdWidth_Neutral:
        case UcdWidth_Narrow:
        case UcdWidth_Halfwidth:
            return 1;
        default:
            nvUnreachable;
        }
    }
}

bool ucdIsCPAlphanumeric(UcdCP cp) {
    // Ascii shortcut
    if (
        (cp >= '0' && cp <= '9')
        || (cp >= 'a' && cp <= 'z')
        || (cp >= 'A' && cp <= 'Z')
    ) {
        return true;
    } else if (!ucdIsCPValid(cp) || cp < 0x80) {
        return false;
    }

    UcdCPInfo info = ucdGetCPInfo(cp);
    return UcdMajorCategory(info.category) == UcdCategory_L
        || UcdMajorCategory(info.category) == UcdCategory_N;
}
