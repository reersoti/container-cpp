# Custom Containers in C++

![CMake CI](https://github.com/reersoti/container-cpp/actions/workflows/cmake.yml/badge.svg)

An educational C++ project that implements custom data structures and container abstractions from scratch.

## Overview

This repository contains several custom container implementations written in C++.  
The goal of the project is to explore how common data structures can be designed without relying entirely on the STL and to better understand internal container organization, iteration, and memory ownership.

The project includes implementations of:

- linked list
- hash-based set
- tree structure
- supporting memory-related abstractions

This repository was created as a practical exercise in low-level programming, abstraction design, and data structure implementation.

## Features

- custom container interfaces
- linked list implementation
- hash-based set
- tree structure
- iterator-style traversal
- basic memory abstraction layer

## Project Structure

```text
.
├── include/        # header files
├── src/            # source files
├── tests/          # smoke tests / examples
├── CMakeLists.txt
└── README.md
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/containers_demo
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

## Continuous Integration

The repository includes a GitHub Actions workflow that automatically builds the project and runs tests on every push and pull request to `main`.

## Educational Value

This project helps practice:

- implementation of classical data structures
- low-level design in C++
- iterator-oriented access patterns
- manual reasoning about memory ownership
- creation of reusable abstractions

## What This Project Demonstrates

- understanding of core data structures
- practical work with pointers and ownership
- abstraction of common container behavior
- separation of interface and implementation
- educational approach to systems-level programming

## Possible Improvements

- add templates for stronger type safety
- extend iterator support
- improve STL-style compatibility
- add more complete tests
- include performance benchmarks
- document internal design decisions in more detail

## Tech Stack

- C++20
- CMake
- CTest
- GitHub Actions

## Notes

This is an educational project focused on understanding container internals and low-level programming concepts.
