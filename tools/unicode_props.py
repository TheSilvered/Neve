from __future__ import annotations

from enum import Enum

class UnicodeEnumProp(Enum):
    @classmethod
    def map(cls) -> dict[str, UnicodeEnumProp]:
        return { w.value: w for w in cls }

    def __repr__(self):
        return self.name.title()


class GeneralCategory(UnicodeEnumProp):
    LETTER_UPPERCASE = "Lu"
    LETTER_LOWERCASE = "Ll"
    LETTER_TITLECASE = "Lt"
    MARK_NON_SPACING = "Mn"
    MARK_SPACING_COMBINING = "Mc"
    MARK_ENCLOSING = "Me"
    NUMBER_DECIMAL_DIGIT = "Nd"
    NUMBER_LETTER = "Nl"
    NUMBER_OTHER = "No"
    SEPARATOR_SPACE = "Zs"
    SEPARATOR_LINE = "Zl"
    SEPARATOR_PARAGRAPH = "Zp"
    OTHER_CONTROL = "Cc"
    OTHER_FORMAT = "Cf"
    OTHER_SURROGATE = "Cs"
    OTHER_PRIVATE_USE = "Co"
    OTHER_NOT_ASSIGNED = "Cn"
    LETTER_MODIFIER = "Lm"
    LETTER_OTHER = "Lo"
    PUNCTUATION_CONNECTOR = "Pc"
    PUNCTUATION_DASH = "Pd"
    PUNCTUATION_OPEN = "Ps"
    PUNCTUATION_CLOSE = "Pe"
    PUNCTUATION_INITIAL_QUOTE = "Pi"
    PUNCTUATION_FINAL_QUOTE = "Pf"
    PUNCTUATION_OTHER = "Po"
    SYMBOL_MATH = "Sm"
    SYMBOL_CURRENCY = "Sc"
    SYMBOL_MODIFIER = "Sk"
    SYMBOL_OTHER = "So"


class BidiClass(UnicodeEnumProp):
    LEFT_TO_RIGHT = "L"
    RIGHT_TO_LEFT = "R"
    ARABIC_LETTER = "AL"
    EUROPEAN_NUMBER = "EN"
    EUROPEAN_SEPARATOR = "ES"
    EUROPEAN_TERMINATOR = "ET"
    ARABIC_NUMBER = "AN"
    COMMON_SEPARATOR = "CS"
    NONSPACING_MARK = "NSM"
    BOUNDARY_NEUTRAL = "BN"
    PARAGRAPH_SEPARATOR = "B"
    SEGMENT_SEPARATOR = "S"
    WHITE_SPACE = "WS"
    OTHER_NEUTRAL = "ON"
    LEFT_TO_RIGHT_EMBEDDING = "LRE"
    LEFT_TO_RIGHT_OVERRIDE = "LRO"
    RIGHT_TO_LEFT_EMBEDDING = "RLE"
    RIGHT_TO_LEFT_OVERRIDE = "RLO"
    POP_DIRECTIONAL_FORMAT = "PDF"
    LEFT_TO_RIGHT_ISOLATE = "LRI"
    RIGHT_TO_LEFT_ISOLATE = "RLI"
    FIRST_STRONG_ISOLATE = "FSI"
    POP_DIRECTIONAL_ISOLATE = "PDI"


class EastAsianWidth(UnicodeEnumProp):
    FULLWIDTH = "F"
    HALFWIDTH = "H"
    WIDE = "W"
    NARROW = "Na"
    AMBIGUOUS = "A"
    NEUTRAL = "N"


class CodePoint(int):
    def __new__(cls, val: int | str):
        return super().__new__(cls, val if isinstance(val, int) else int(val, 16))

    def __repr__(self):
        return f"{self:04X}"


class CodePointRange:
    def __init__(self, first: CodePoint, last: CodePoint):
        self.first = first
        self.last = last

    def cp_in_range(self, cp: CodePoint):
        return cp >= self.first and cp <= self.last

    def __repr__(self):
        return f"{self.first}..{self.last}"


class DecompositionTag(UnicodeEnumProp):
    FONT = "<font>"
    NOBREAK = "<noBreak>"
    INITIAL = "<initial>"
    MEDIAL = "<medial>"
    FINAL = "<final>"
    ISOLATED = "<isolated>"
    CIRCLE = "<circle>"
    SUPER = "<super>"
    SUB = "<sub>"
    VERTICAL = "<vertical>"
    WIDE = "<wide>"
    NARROW = "<narrow>"
    SMALL = "<small>"
    SQUARE = "<square>"
    FRACTION = "<fraction>"
    COMPAT = "<compat>"


class Decomposition:
    def __init__(self, tag: DecompositionTag | None, mapping: list[CodePoint]):
        self.tag = tag
        self.mapping = mapping

    def __repr__(self):
        if self.tag is not None:
            return f"{self.tag} {' '.join(str(cp) for cp in self.mapping)}"
        else:
            return ' '.join(str(cp) for cp in self.mapping)
