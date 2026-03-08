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
- [ ] Pretty Printer
- [ ] CLI
- [ ] Query Engine
- [ ] Schema Validator
- [ ] Benchmarks

## Build

\```bash
mkdir build && cd build
cmake ..
cmake --build .
\```

## Usage

Coming soon.

## Tech

- C++17
- CMake
- Catch2 (tests)
- Google Benchmark (benchmarks)
