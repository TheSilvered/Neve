from __future__ import annotations

from unicode_props import *
from unicode_parser import *
from table_generator import TableGenerator


def load_east_asian_width() -> list[dict[str, Any]]:
    parser = Parser(
        range=CodePointRangeField(),
        width=UnicodeEnumPropField(EastAsianWidth)
    )
    with open("ucd-16.0.0/EastAsianWidth.txt", encoding="utf8") as f:
        east_asian_width = parser.parse(f)

    #   - The unassigned code points in the following blocks default to "W":
    #         CJK Unified Ideographs Extension A: U+3400..U+4DBF
    #         CJK Unified Ideographs:             U+4E00..U+9FFF
    #         CJK Compatibility Ideographs:       U+F900..U+FAFF
    #  - All undesignated code points in Planes 2 and 3, whether inside or
    #      outside of allocated blocks, default to "W":
    #         Plane 2:                            U+20000..U+2FFFD
    #         Plane 3:                            U+30000..U+3FFFD

    # Adding default Wide codepoints
    east_asian_width.insert(0, {
        "range": CodePointRange(CodePoint(0x3400), CodePoint(0x4dbf)),
        "width": EastAsianWidth.WIDE
    })
    east_asian_width.insert(0, {
        "range": CodePointRange(CodePoint(0x4e00), CodePoint(0x9fff)),
        "width": EastAsianWidth.WIDE
    })
    east_asian_width.insert(0, {
        "range": CodePointRange(CodePoint(0xf900), CodePoint(0xfaff)),
        "width": EastAsianWidth.WIDE
    })
    east_asian_width.insert(0, {
        "range": CodePointRange(CodePoint(0x20000), CodePoint(0x2fffd)),
        "width": EastAsianWidth.WIDE
    })
    east_asian_width.insert(0, {
        "range": CodePointRange(CodePoint(0x30000), CodePoint(0x3fffd)),
        "width": EastAsianWidth.WIDE
    })

    return east_asian_width


def load_special_casing() -> list[dict[str, Any]]:
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


def load_unicode_data() -> list[dict[str, Any]]:
    parser = Parser(
        cp=CodePointRangeField(),
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
        data = parser.parse(f)

    unicode_data: list[dict[str, Any]] = []
    range_start = None

    # Normalize ranges
    for line in data:
        if ", First" in line["name"]:
            range_start = line
        elif ", Last" in line["name"] and range_start is not None:
            range_start["cp"].last = line["cp"].first
            unicode_data.append(range_start)
            range_start = None
        else:
            unicode_data.append(line)

    return unicode_data


type UdbCPInfo = tuple[GeneralCategory, EastAsianWidth]


def udb_cp_info_formatter(data: UdbCPInfo):
    return f"{{ UdbCategory_{data[0].value}, UdbWidth_{data[1]!r} }}"


def create_cp_info_list(
    unicode_data: list[dict[str, Any]],
    east_asian_width: list[dict[str, Any]]
) -> tuple[list[int], list[UdbCPInfo]]:
    unique_values: dict[UdbCPInfo, int] = {
        (GeneralCategory.OTHER_NOT_ASSIGNED, EastAsianWidth.NEUTRAL,): 0  # default value
    }
    print("Generating default list")
    indices: list[int] = [0] * 0x10ffff
    properties: list[dict[str, Any]] = [{} for _ in range(0x10ffff)]

    print("Generating properties")
    for prop in PropIterator(unicode_data, "cp"):
        cp: CodePoint = prop["cp"]
        properties[cp]["category"] = prop["category"]

    for prop in PropIterator(east_asian_width, "range"):
        cp: CodePoint = prop["range"]
        properties[cp]["width"] = prop["width"]

    print("Assigning indices")
    for cpIdx, prop in enumerate(properties):
        value: UdbCPInfo = (
            prop.get("category", GeneralCategory.OTHER_NOT_ASSIGNED),
            prop.get("width", EastAsianWidth.NEUTRAL)
        )
        idx = unique_values.get(value)
        if idx is None:
            idx = len(unique_values)
            unique_values[value] = len(unique_values)
        indices[cpIdx] = idx

    return indices, list(unique_values.keys())


def indices_datatype(max_idx):
    if max_idx < 2**8:
        return "uint8_t"
    elif max_idx < 2**16:
        return "uint16_t"
    elif max_idx < 2**32:
        return "uint32_t"
    else:
        return "uint64_t"


def main():
    east_asian_width = load_east_asian_width()
    # special_casing = load_special_casing()
    unicode_data = load_unicode_data()

    indices, data = create_cp_info_list(unicode_data, east_asian_width)

    generator = TableGenerator("../src/nv_udb_tables.c")
    generator.writeln('/* File generated by ucd_gen.py */')
    generator.writeln('#include "nv_udb.h"')
    generator.writeln()

    generator.add_array("g_cpInfo", "UdbCPInfo", data, udb_cp_info_formatter)
    generator.add_array("g_infoIndices", indices_datatype(len(data)), indices)

    generator.save_file()


if __name__ == "__main__":
    main()
