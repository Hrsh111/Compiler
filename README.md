# Compiler

A compiler implementation written in C that performs **lexical analysis** and **syntax analysis** (parsing) for a custom programming language.

## Overview

This project implements a compiler front-end with the following components:

- **Lexical Analyzer (Lexer)**: Tokenizes source code using a twin-buffer technique
- **Syntax Analyzer (Parser)**: Performs LL(1) parsing with automated FIRST and FOLLOW set computation
- **Parse Table Generator**: Automatically constructs the parse table from grammar rules

## Project Structure

```
├── driver.c          # Main driver program with menu interface
├── lexer.c           # Lexical analyzer implementation
├── lexer.h           # Lexer function declarations
├── lexerDef.h        # Token definitions and data structures
├── parser.c          # Syntax analyzer implementation
├── parser.h          # Parser function declarations
├── parserDef.h       # Parser data structures
├── stack.c           # Stack implementation for parsing
├── stack.h           # Stack declarations
├── grammar.txt       # Grammar rules for the language
├── makefile          # Build configuration
└── TestCases/        # Test input files
```

## Building the Project

### Prerequisites
- GCC compiler
- Make utility

### Compilation

```bash
make
```

This will compile all source files and create the `myCompiler` executable.

### Clean Build

```bash
make clean
```

## Usage

```bash
./myCompiler <input_source_file> <output_parse_tree_file>
```

### Menu Options

| Option | Description |
|--------|-------------|
| 0 | Exit the program |
| 1 | Remove comments and print comment-free code |
| 2 | Print token list (lexer output) |
| 3 | Parse input source code and generate parse tree |
| 4 | Print total CPU time taken by lexer and parser |

## Language Grammar

The compiler supports a custom language with the following constructs:

- **Functions**: Defined with `_funid` naming convention
- **Data Types**: `int`, `real`, `record`, `union`
- **Control Flow**: `if-then-else-endif`, `while-endwhile`
- **I/O Operations**: `read`, `write`
- **Expressions**: Arithmetic and boolean expressions
- **Type Definitions**: Records and unions

## Token Types

The lexer recognizes 59 token types including:
- Operators: `+`, `-`, `*`, `/`, `<`, `>`, `<=`, `>=`, `==`, `!=`
- Keywords: `_main`, `if`, `then`, `else`, `while`, `read`, `write`, `return`, etc.
- Identifiers: Variables (`TK_ID`), Functions (`TK_FUNID`), Records (`TK_RUID`)
- Literals: Integers (`TK_NUM`), Real numbers (`TK_RNUM`)
- Delimiters: `;`, `:`, `,`, `(`, `)`, `[`, `]`

## Implementation Status

- [x] FIRST and FOLLOW set computation (automated)
- [x] Lexical analyzer module
- [x] Syntax analysis module
- [x] Successful compilation (no segmentation faults)
- [x] Compatible with test cases 2, 3, and 4
- [ ] Parse tree printing (not implemented)

## Testing

Test cases are provided in the `TestCases/` directory. Run them using:

```bash
./myCompiler TestCases/testcase1.txt output.txt
```

## Technical Details

### Twin Buffer Technique
The lexer uses a twin-buffer approach with a buffer size of 4096 bytes for efficient file reading.

### Hash Table
A hash table with size 2^20 is used for the symbol table implementation.

### LL(1) Parsing
The parser implements LL(1) top-down parsing with:
- Automated FIRST set computation
- Automated FOLLOW set computation
- Parse table construction

## License

This project is open source and available for educational purposes.

## Author

[@Hrsh111](https://github.com/Hrsh111)