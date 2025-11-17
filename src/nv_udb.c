#include <assert.h>
#include "nv_udb.h"

#include "nv_udb_tables.c"
#define blockIdxMask_ ((1<<udbShift_) - 1)

UdbCPInfo udbGetCPInfo(UcdCP cp) {
    assert(cp <= UcdCPMax);
    assert(cp >= 0);
    if (cp < 0 || cp > UcdCPMax) {
        cp = 0;
    }
    uint8_t blockIdx = g_blockIndices[cp >> udbShift_];
    uint8_t cpInfoIdx = g_infoBlocks[
        (blockIdx << udbShift_) + (cp & blockIdxMask_)
    ];

    return g_cpInfo[cpInfoIdx];
}
