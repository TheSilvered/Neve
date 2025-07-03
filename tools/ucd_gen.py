from __future__ import annotations

from pprint import pprint

from unicode_props import *
from unicode_parser import *


def load_east_asian_width() -> list[list[CodePointRange, EastAsianWidth]]:
    parser = Parser(
        range=CodePointRangeField(),
        width=UnicodeEnumPropField(EastAsianWidth)
    )
    with open("ucd-16.0.0/EastAsianWidth.txt", encoding="utf8") as f:
        east_asian_width = parser.parse(f)
    return east_asian_width


def load_special_casing() -> list[list[CodePoint, list[CodePoint], list[CodePoint], list[CodePoint]]]:
    parser = Parser(
        cp=CodePointField(),
        lower=CodePointSequenceField(),
        title=CodePointSequenceField(),
        upper=CodePointSequenceField(),
        condition=StrField()
    )
    with open("ucd-16.0.0/SpecialCasing.txt", encoding="utf8") as f:
        special_casing = parser.parse(f)

    return [(line.pop("condition"), line)[1] for line in special_casing if not line["condition"]]


def load_unicode_data() -> list[list[Any]]:
    parser = Parser(
        cp=CodePointField(),
        name=StrField(),
        category=UnicodeEnumPropField(GeneralCategory),
        combining_class=IntField(),
        bidi_class=UnicodeEnumPropField(BidiClass),
        decomposition=OptionalField(DecompositionField()),
        decimal_value=OptionalField(IntField()),
        decimal_value_special=OptionalField(IntField()),
        numeric_value=OptionalField(FractionField()),
        bidi_mirrored=BooleanField(),
        unicode1_name=StrField(),
        iso_comment=IgnoreField(),
        simple_uppercase=OptionalField(CodePointField()),
        simple_lowercase=OptionalField(CodePointField()),
        simple_titlecase=OptionalField(CodePointField())
    )
    with open("ucd-16.0.0/UnicodeData.txt", encoding="utf8") as f:
        unicode_data = parser.parse(f)
    return unicode_data


def main():
    east_asian_width = load_east_asian_width()
    special_casing = load_special_casing()
    unicode_data = load_unicode_data()


if __name__ == "__main__":
    main()
