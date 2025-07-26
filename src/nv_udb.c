#include <assert.h>
#include "nv_udb.h"

#include "nv_udb_tables.c"

UdbCPInfo udbGetCPInfo(UcdCP cp) {
    assert(cp <= 0x10ffff);
    return g_cpInfo[g_infoIndices[cp]];
}
