#ifndef NV_UCD_H_
#define NV_UCD_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define ucdHighSurrogateFirst 0xD800
#define ucdHighSurrogateLast  0xDBFF

#define ucdLowSurrogateFirst 0xDC00
#define ucdLowSurrogateLast  0xDFFF

#define ucdCPMax 0x10FFFF

// Get the major category from a category.
#define UcdMajorCategory(category) ((category) & 0xF0)
// Check if category is in `UcdCategory_LC`
#define UcdIsCategoryLC(category) (((category) & 0xF8) == UcdCategory_LC)

// Unicode General_Category property.
// 8-bit field. Top four bits are the major category, bottom four the minor.
enum UcdCategory {
    UcdCategory_Lu = 0x19, // Letter, uppercase 0001 1 001
    UcdCategory_Ll = 0x1a, // Letter, lowercase 0001 1 010
    UcdCategory_Lt = 0x1b, // Letter, titlecase 0001 1 011
    UcdCategory_Lm = 0x11, // Letter, modifier  0001 0 001
    UcdCategory_Lo = 0x12, // Letter, other     0001 0 010

    UcdCategory_Mn = 0x21, // Mark, nonspacing 0010 0001
    UcdCategory_Mc = 0x22, // Mark, combining  0010 0010
    UcdCategory_Me = 0x23, // Mark, enclosing  0010 0011

    UcdCategory_Nd = 0x31, // Number, digit  0011 0001
    UcdCategory_Nl = 0x32, // Number, letter 0011 0010
    UcdCategory_No = 0x33, // Number, other  0011 0011

    UcdCategory_Pc = 0x41, // Punctuation, connector     0100 0001
    UcdCategory_Pd = 0x42, // Punctuation, dash          0100 0010
    UcdCategory_Ps = 0x43, // Punctuation, open          0100 0011
    UcdCategory_Pe = 0x44, // Punctuation, close         0100 0100
    UcdCategory_Pi = 0x45, // Punctuation, initial quote 0100 0101
    UcdCategory_Pf = 0x46, // Punctuation, final quote   0100 0110
    UcdCategory_Po = 0x47, // Punctuation, other         0100 0111

    UcdCategory_Sm = 0x51, // Symbol, math     0101 0001
    UcdCategory_Sc = 0x52, // Symbol, currency 0101 0010
    UcdCategory_Sk = 0x53, // Symbol, modifier 0101 0011
    UcdCategory_So = 0x54, // Symbol, other    0101 0100

    UcdCategory_Zs = 0x61, // Separator, space     0110 0001
    UcdCategory_Zl = 0x62, // Separator, line      0110 0010
    UcdCategory_Zp = 0x63, // Separator, paragraph 0110 0011

    UcdCategory_Cc = 0x71, // Other, control      0111 0001
    UcdCategory_Cf = 0x72, // Other, format       0111 0010
    UcdCategory_Cs = 0x73, // Other, surrogate    0111 0011
    UcdCategory_Co = 0x74, // Other, private use  0111 0100
    UcdCategory_Cn = 0x75, // Other, not assigned 0111 0101

    UcdCategory_LC = 0x18, // Letter, cased (use `UcdIsCategoryLC`)
    UcdCategory_L  = 0x10, // Letter (use with `UcdMajorCategory`)
    UcdCategory_M  = 0x20, // Mark (use with `UcdMajorCategory`)
    UcdCategory_N  = 0x30, // Number (use with `UcdMajorCategory`)
    UcdCategory_P  = 0x40, // Punctuation (use with `UcdMajorCategory`)
    UcdCategory_S  = 0x50, // Symbol (use with `UcdMajorCategory`)
    UcdCategory_Z  = 0x60, // Separator (use with `UcdMajorCategory`)
    UcdCategory_C  = 0x70, // Other (use with `UcdMajorCategory`)
};

typedef uint8_t UcdCategory;

// Unicode East_Asian_Width property
typedef enum UcdWidth {
    UcdWidth_Fullwidth,
    UcdWidth_Wide,
    UcdWidth_Halfwidth,
    UcdWidth_Narrow,
    UcdWidth_Neutral,
    UcdWidth_Ambiguous
} UcdWidth;

typedef struct UcdCPInfo {
    UcdCategory category;
    UcdWidth width;
} UcdCPInfo;

// Unicode Codepoint.
typedef int32_t UcdCP;

// Get the info for a particular codepoint.
UcdCPInfo ucdGetCPInfo(UcdCP cp);

// Check if a codepoint is valid.
// A valid codepoint is not a high or low surrogate and is not above U+10FFFF.
bool ucdIsCPValid(UcdCP cp);

// Check if a codepoint is a noncharacter.
bool ucdIsCPNoncharacter(UcdCP cp);

// Get the width of a character (based on the East_Asian_Width property).
// If `currWidth` is `0` the width of `\t` will always be `tabStop`.
uint8_t ucdCPWidth(UcdCP cp, uint8_t tabStop, size_t currWidth);

// Check if a character is alphanumeric.
bool ucdIsCPAlphanumeric(UcdCP cp);
// Check if a character is white space.
bool ucdIsCPWhiteSpace(UcdCP cp);

#endif // !NV_UCD_H_
