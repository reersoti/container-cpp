# Design Notes

This document summarizes the design intent behind the custom container implementations.

## Purpose

The project is educational. Its goal is to better understand how common containers can be implemented internally instead of relying only on STL abstractions.

## Main Concepts

- linked structures and pointer-based ownership;
- hash-based lookup logic;
- tree-like data organization;
- iterator-style traversal;
- separation between public container behavior and internal storage details.

## Design Principles

### Keep the API Small

The project should expose only the operations required to demonstrate the data structure clearly.

### Prefer Readable Educational Code

The implementation should favor readability and explicit logic over excessive abstraction.

### Separate Examples from Core Logic

Demo code should stay in `src/main.cpp`, while reusable data structure logic should stay in headers or dedicated source files.

### Test Behavior, Not Implementation Details

Tests should check public behavior of containers rather than depending too much on private internals.

## Future Improvements

- add template support where appropriate;
- expand iterator compatibility;
- add more edge-case tests;
- add simple benchmark examples;
- document complexity of key operations.
