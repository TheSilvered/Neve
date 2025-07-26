from typing import Callable

class TableGenerator:
    def __init__(self, out_file: str, max_line_length: int = 80, indent: str = "    "):
        self.out_file = out_file
        self.max_line_length = max_line_length
        self.indent_level: int = 0
        self.indent_literal = indent
        self.file_content: str = ""

    def indent(self):
        self.indent_level += 1

    def dedent(self):
        assert self.indent_level >= 1
        self.indent_level -= 1

    def line_length(self, line_content: str) -> int:
        return len(line_content) + len(self.indent_literal) * self.indent_level

    def writeln(self, line_content: str = ""):
        self.file_content += self.indent_literal * self.indent_level
        self.file_content += line_content
        self.file_content += "\n"

    def add_array(self, name: str, data_type: str, data: list, formatter: Callable = str):
        self.writeln(f"{data_type} {name}[{len(data)}] = {{")
        self.indent()
        data_strings = [formatter(element) for element in data]
        line = ""
        for s in data_strings:
            if self.line_length(line + f" {s},") > self.max_line_length and line:
                self.writeln(line)
                line = ""
            line += f" {s},"
        if line:
            self.writeln(line)
        self.dedent()
        self.writeln("};")
        self.writeln()

    def save_file(self):
        with open(self.out_file, "w", encoding="utf8") as f:
            f.write(self.file_content)
