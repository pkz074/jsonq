# jsonq

A jq-inspired JSON query engine written in C++ from scratch.

## Overview

jsonq is a command-line tool that parses JSON files and lets you extract,
filter, and pretty-print data using a simple query language built without
any parsing libraries.

## Status

Work in progress. Being built layer by layer:

- [x] Lexer
- [x] Parser + AST
- [x] Pretty Printer
- [x] Basic CLI for reading and pretty-printing a JSON file
- [x] Basic Query Engine for object keys and array indices
- [ ] Schema Validator
- [ ] Benchmarks

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

Pretty-print a JSON file:

```bash
./build/jsonq sample.json
```

Select values with a query:

```bash
./build/jsonq sample.json '$.name'
./build/jsonq sample.json '$.hobbies[1]'
./build/jsonq sample.json '$.hobbies[*]'
```

Supported query syntax:

- `$` selects the whole document.
- `.key` selects an object field.
- `[n]` selects an array element by zero-based index.
- `[*]` selects every element from an array and returns an array.
- These can be combined, for example `$.user.hobbies[0]`.
- Path steps after `[*]` are applied to every selected element, for example
  `$.users[*].name`.

The JSON lexer supports common string escapes such as `\"`, `\\`, `\/`, `\n`,
`\r`, `\t`, `\b`, and `\f`, plus Unicode escapes like `\u0041` and surrogate
pairs like `\uD83D\uDE00`.

## Tests

```bash
ctest --test-dir build
```

You can also run individual test binaries:

```bash
./build/test_lexer
./build/test_parser
./build/test_query
./build/test_printer
```

## Tech

- C++17
- CMake
- Catch2 (tests)
