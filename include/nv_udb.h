#ifndef NV_UDB_H_
#define NV_UDB_H_

#include <inttypes.h>
#include "nv_unicode.h"

// Get the major category from a category.
#define UdbMajorCategory(category) ((category) & 0xF0)
// Check if category is in `UdbCategory_LC`
#define UdbIsCategoryLC(category) (((category) & 0xF8) == UdbCategory_LC)

// Unicode General_Category property.
// 8-bit field. Top four bits are the major category, bottom four the minor.
enum UdbCategory {
    UdbCategory_Lu = 0x19, // Letter, uppercase 0001 1 001
    UdbCategory_Ll = 0x1a, // Letter, lowercase 0001 1 010
    UdbCategory_Lt = 0x1b, // Letter, titlecase 0001 1 011
    UdbCategory_Lm = 0x11, // Letter, modifier  0001 0 001
    UdbCategory_Lo = 0x12, // Letter, other     0001 0 010

    UdbCategory_Mn = 0x21, // Mark, nonspacing 0010 0001
    UdbCategory_Mc = 0x22, // Mark, combining  0010 0010
    UdbCategory_Me = 0x23, // Mark, enclosing  0010 0011

    UdbCategory_Nd = 0x31, // Number, digit  0011 0001
    UdbCategory_Nl = 0x32, // Number, letter 0011 0010
    UdbCategory_No = 0x33, // Number, other  0011 0011

    UdbCategory_Pc = 0x41, // Punctuation, connector     0100 0001
    UdbCategory_Pd = 0x42, // Punctuation, dash          0100 0010
    UdbCategory_Ps = 0x43, // Punctuation, open          0100 0011
    UdbCategory_Pe = 0x44, // Punctuation, close         0100 0100
    UdbCategory_Pi = 0x45, // Punctuation, initial quote 0100 0101
    UdbCategory_Pf = 0x46, // Punctuation, final quote   0100 0110
    UdbCategory_Po = 0x47, // Punctuation, other         0100 0111

    UdbCategory_Sm = 0x51, // Symbol, math     0101 0001
    UdbCategory_Sc = 0x52, // Symbol, currency 0101 0010
    UdbCategory_Sk = 0x53, // Symbol, modifier 0101 0011
    UdbCategory_So = 0x54, // Symbol, other    0101 0100

    UdbCategory_Zs = 0x61, // Separator, space     0110 0001
    UdbCategory_Zl = 0x62, // Separator, line      0110 0010
    UdbCategory_Zp = 0x63, // Separator, paragraph 0110 0011

    UdbCategory_Cc = 0x71, // Other, control      0111 0001
    UdbCategory_Cf = 0x72, // Other, format       0111 0010
    UdbCategory_Cs = 0x73, // Other, surrogate    0111 0011
    UdbCategory_Co = 0x74, // Other, private use  0111 0100
    UdbCategory_Cn = 0x75, // Other, not assigned 0111 0101

    UdbCategory_LC = 0x18, // Letter, cased (use `UdbIsCategoryLC`)
    UdbCategory_L  = 0x10, // Letter (use with `UdbMajorCategory`)
    UdbCategory_M  = 0x20, // Mark (use with `UdbMajorCategory`)
    UdbCategory_N  = 0x30, // Number (use with `UdbMajorCategory`)
    UdbCategory_P  = 0x40, // Punctuation (use with `UdbMajorCategory`)
    UdbCategory_S  = 0x50, // Symbol (use with `UdbMajorCategory`)
    UdbCategory_Z  = 0x60, // Separator (use with `UdbMajorCategory`)
    UdbCategory_C  = 0x70, // Other (use with `UdbMajorCategory`)
};

typedef uint8_t UdbCategory;

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
