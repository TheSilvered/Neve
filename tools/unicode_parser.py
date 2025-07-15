from abc import ABC, abstractmethod
from typing import Any, TextIO
from itertools import zip_longest
from fractions import Fraction

from unicode_props import *

class Field(ABC):
    @abstractmethod
    def parse(self, content: str | None) -> Any:
        pass


class Parser:
    def __init__(self, **fields: Field):
        self.fields: dict[str, Field] = fields

    def parse(self, file: TextIO) -> list[dict[str, Any]]:
        content = file.read()
        lines = content.replace("\r\n", "\n").replace("\r", "\n").split("\n")
        del content

        file_result = []
        for line_no, line in enumerate(lines):
            line = line.split("#")[0]
            if len(line) == 0:
                continue
            line_result = {}
            for field, content in zip_longest(self.fields.items(), line.split(";")):
                if field is None:
                    break
                field_name, field_parser = field
                if content is not None:
                    content = content.strip()
                try:
                    result = field_parser.parse(content)
                except Exception as e:
                    e.add_note(f"{file.name}:{line_no + 1}, field {field_name}")
                    raise e
                line_result[field_name] = result
            file_result.append(line_result)
        return file_result


class CodePointField(Field):
    def parse(self, content: str | None) -> CodePoint:
        if content is None:
            raise ValueError("content required")
        return CodePoint(content)


class CodePointRangeField(Field):
    def parse(self, content: str | None) -> CodePointRange:
        if content is None:
            raise ValueError("content required")
        if ".." in content:
            return CodePointRange(*(CodePoint(x) for x in content.split("..")))
        else:
            val = CodePoint(content)
            return CodePointRange(val, val)


class CodePointSequenceField(Field):
    def parse(self, content: str | None) -> list[CodePoint]:
        if content is None:
            raise ValueError("content required")
        return [CodePoint(x) for x in content.split()]


class UnicodeEnumPropField(Field):
    def __init__(self, prop: type[UnicodeEnumProp]):
        self.prop_map = prop.map()

    def parse(self, content: str | None) -> UnicodeEnumProp:
        if content in self.prop_map:
            return self.prop_map[content]
        else:
            raise ValueError(f"invalid option {content!r}")


class StrField(Field):
    def parse(self, content: str | None) -> str:
        if content is None:
            raise ValueError("content required")
        return content


class IntField(Field):
    def parse(self, content: str | None) -> int:
        if content is None:
            raise ValueError("content required")
        return int(content)


class FractionField(Field):
    def parse(self, content: str | None) -> Fraction:
        if content is None:
            raise ValueError("content required")
        return Fraction(content)


class DecompositionField(Field):
    def parse(self, content: str | None) -> Decomposition:
        if content is None:
            raise ValueError("content required")
        if "<" in content:
            tag, mapping = content.split(maxsplit=1)
        else:
            tag = None
            mapping = content
        return Decomposition(
            DecompositionTag.map().get(tag) if tag is not None else None,
            [CodePoint(x) for x in mapping.split()]
        )


class BooleanField(Field):
    def __init__(self, true_variant="Y", false_variant="N"):
        self.true_variant = true_variant
        self.false_variant = false_variant

    def parse(self, content: str | None) -> bool:
        if content == self.true_variant:
            return True
        elif content == self.false_variant:
            return False
        else:
            raise ValueError(f"expected {self.true_variant!r} or {self.false_variant!r}, found {content!r}")


class OptionalField(Field):
    def __init__(self, field: Field, default_value: Any = None):
        self.field = field
        self.default_value = default_value

    def parse(self, content: str | None) -> Any:
        if content is None or not content:
            return self.default_value
        else:
            return self.field.parse(content)


class IgnoreField(Field):
    def parse(self, content: str | None) -> None:
        return None
