#ifndef NV_UDB_H_
#define NV_UDB_H_

#include <inttypes.h>
#include "nv_unicode.h"

// Unicode General_Category property
typedef enum UdbCategory {
    UdbCategory_Lu,
    UdbCategory_Ll,
    UdbCategory_Lt,
    UdbCategory_Lm,
    UdbCategory_Lo,

    UdbCategory_Mn,
    UdbCategory_Mc,
    UdbCategory_Me,

    UdbCategory_Nd,
    UdbCategory_Nl,
    UdbCategory_No,

    UdbCategory_Pc,
    UdbCategory_Pd,
    UdbCategory_Ps,
    UdbCategory_Pe,
    UdbCategory_Pi,
    UdbCategory_Pf,
    UdbCategory_Po,

    UdbCategory_Sm,
    UdbCategory_Sc,
    UdbCategory_Sk,
    UdbCategory_So,

    UdbCategory_Zs,
    UdbCategory_Zl,
    UdbCategory_Zp,

    UdbCategory_Cc,
    UdbCategory_Cf,
    UdbCategory_Cs,
    UdbCategory_Co,
    UdbCategory_Cn,

    UdbCategory_LC_First = UdbCategory_Lu,
    UdbCategory_LC_Last = UdbCategory_Lt,
    UdbCategory_L_First = UdbCategory_Lu,
    UdbCategory_L_Last = UdbCategory_Lo,
    UdbCategory_M_First = UdbCategory_Mn,
    UdbCategory_M_Last = UdbCategory_Me,
    UdbCategory_N_First = UdbCategory_Nd,
    UdbCategory_N_Last = UdbCategory_No,
    UdbCategory_P_First = UdbCategory_Pc,
    UdbCategory_P_Last = UdbCategory_Po,
    UdbCategory_S_First = UdbCategory_Sm,
    UdbCategory_S_Last = UdbCategory_So,
    UdbCategory_Z_First = UdbCategory_Zs,
    UdbCategory_Z_Last = UdbCategory_Zp,
    UdbCategory_C_First = UdbCategory_Cc,
    UdbCategory_C_Last = UdbCategory_Cn,
} UdbCategory;

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
    UdbCategory category;
    UdbWidth width;
} UdbCPInfo;

UdbCPInfo udbGetCPInfo(UcdCP cp);

#endif // !NV_UDB_H_
