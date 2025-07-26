# Directory structure

```text
.github/
    workflows/        # Github actions
include/              # Header files for the editor
src/                  # Source of the editor
tests/
    main/             # Test "library"
    test_nv_string/   # `nv_string.h` tests
    test_nv_unicode/  # `nv_unicode.h` tests
tools/                # Miscellaneous scripts
```

# Coding style

## Types

### Integers

Prefer integers defined in `stdint.h` with a known width and signedness. Use
regular types only if they are passed to or a the result of a system function.

### Strings

- `UcdCh8 *` is a UTF-8 encoded string, it may not have a `NUL` terminator.
- `UcdCh16 *` is a UTF-16 encoded string, it may not have a `NUL` terminator.
- `UcdCh32 *` is a UTF-32 encoded string, it may not have a `NUL` terminator.
- `char *` is a UTF-8 encoded and `NUL`-terminated string.

Strings other than `char *` may contain `NUL` characters.

Prefer `Str`, `StrView` and `StrBuf` to raw C strings. C strings should be used
only in static string literals.

### Constants

If a function does not modify the contents of a pointer they should be marked
as constant.

```c
// CORRECT
// `strAppend` only reads `sv`, it does not modify it
bool strAppend(Str *str, const StrView *sv)

// INCORRECT
bool strAppend(Str *str, StrView *sv)
```

## Explicit ignore of return values

Any return value that is ignored must be marked with a `void` cast.

```c
// CORRECT
#include <stdio.h>

int main(void) {
    (void)printf("Hello, world!\n");
    return 0;
}

// INCORRECT
#include <stdio.h>

int main(void) {
    printf("Hello, world!\n");
    return 0;
}
```

## Style

### Indentation

Use 4-space intentation.

### Line length

Lines should be at most **80 characters long**. You can split lines at
function calls or long operator chains. A line that is too long due to too many
indentatio levels is a signal that the code should probably be re-written.

### Functions

Function definitions:

```c
// CORRECT

void myFunc(int32_t arg1, size_t arg2);

ReallyLongReturnType reallyLongFunctionName(
    LongArgumentType longArgName,
    AnotherArgType *anotherArg
);

// INCORRECT
void myFunc( int32_t arg1, size_t arg2 );

void myFunc(
    int32_t arg1,
    size_t arg2
);

void
myFunc(int32_t arg1, size_t arg2);

ReallyLongReturnType reallyLongFunctionName(LongArgumentType longArgName,
    AnotherArgType *anotherArg);
```

Function calls:

```c
// CORRECT

myFunc(arg1, arg2, arg3);

myFureallyLongFunctionNamenc(
    longArgName,
    anotherArg,
    yetAnotherArgument,
    aLotOfArguments
);

// INCORRECT

myFunc (arg1, arg2, arg3);
myFunc( arg1, arg2, arg3 );
myFunc(arg1 , arg2 , arg3);

myFureallyLongFunctionNamenc(longArgName, anotherArg, yetAnotherArgument,
    aLotOfArguments);

myFureallyLongFunctionNamenc(longArgName,
                             anotherArg,
                             yetAnotherArgument,
                             aLotOfArguments);
```

### Other statements

Always use curly braces even when they are not necessary.

An example for `for` loops, the same applies when dealing with `if` statements,
`while` loops etc.

```c
// CORRECT

for (int32_t i = 0; i < size; i++) {
    arr[i] = obj;
}

for (
    size_t idx = myIterator(&obj, -1, &out);
    idx > 0;
    idx = myIterator(&obj, idx, &out)
) {
    ...
}

// INCORRECT

for (int32_t i = 0; i < size; i++)
    arr[i] = obj;

for (int32_t i = 0; i < size; i++)
{
    arr[i] = obj;
}

for (size_t idx = myIterator(&obj, -1, &out);
     idx > 0;
     idx = myIterator(&obj, idx, &out))
{
    ...
}
```
