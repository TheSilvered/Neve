# Not Emacs or Vim Editor

[![Build and test](https://github.com/TheSilvered/Neve/actions/workflows/build_and_tests.yml/badge.svg)](https://github.com/TheSilvered/Neve/actions/workflows/build_and_tests.yml)

## Building

Build using CMake, Ninja is the preferred generator but Visual Studio project
files and Make files work too. Supported compilers are `cl.exe` and `clang` on
Windows, `clang` and `gcc` on unix.

```sh
cmake -S . -B build/ -G Ninja
cmake --build build --target neve
```
