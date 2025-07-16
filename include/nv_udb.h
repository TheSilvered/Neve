#ifndef NV_UDB_H_
#define NV_UDB_H_

#include <inttypes.h>
#include "nv_unicode.h"

// Unicode East_Asian_Width property
typedef enum UdbWidth {
    UdbWidth_Fullwidth,
    UdbWidth_Wide,
    UdbWidth_Halfwidth,
    UdbWidth_Narrow,
    UdbWidth_Neutral,
    UdbWidth_Ambiguous
} UdbWidth;

typedef struct UdbCPInfo {
    UdbWidth width;
} UdbCPInfo;

UdbCPInfo udbGetCPInfo(UcdCP cp);

#endif // !NV_UDB_H_
