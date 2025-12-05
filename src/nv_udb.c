#include <assert.h>
#include "nv_udb.h"

#include "nv_udb_tables.c"
#define _blockIdxMask ((1<<_udbShift) - 1)

UdbCPInfo udbGetCPInfo(UcdCP cp) {
    assert(cp <= ucdCPMax);
    assert(cp >= 0);
    if (cp < 0 || cp > ucdCPMax) {
        cp = 0;
    }
    uint8_t blockIdx = g_blockIndices[cp >> _udbShift];
    uint8_t cpInfoIdx = g_infoBlocks[
        (blockIdx << _udbShift) + (cp & _blockIdxMask)
    ];

    return g_cpInfo[cpInfoIdx];
}
