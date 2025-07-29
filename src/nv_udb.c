#include <assert.h>
#include "nv_udb.h"

#include "nv_udb_tables.c"

UdbCPInfo udbGetCPInfo(UcdCP cp) {
    assert(cp <= 0x10ffff);
    assert(cp >= 0);
    if (cp < 0 || cp > 0x10ffff) {
        cp = 0;
    }
    return g_cpInfo[g_infoIndices[cp]];
}
