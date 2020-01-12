# SPL

## What is it?
This is the project of CS323 compiler. There are four project totally:
- Lexical Analysis && Syntax Analysis
- Semantic Analysis
- Intermediate Code Generation (TAC)
- Target Code Generation (MIPS)

SPL is a simple language similar to C without malloc, pointer.

## How to use it?

using make file to do simple test.
- `make splc`: compile the source code of spl compiler
- `make test`: generate mips code of spl file in the fold of test
- `make ir`: using irsim run ir test
- `make asm`: using spim run mips test


## To be continue
- Could add string constants in the front end part.
- In the intermediate code generation part, it doesn't implements scope variables. The float and char support is not enought, so string not supported.
- In the target code generation part, it doesn't implement struct and array.
