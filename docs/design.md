# CudaGuard Design Document

## Overview

CudaGuard is a static analysis tool for CUDA C++ source code. It uses Clang LibTooling and AST Matchers to detect common CUDA programming mistakes before compilation.

## Architecture

### High-Level Flow

```
Source Files (.cu)  ──→  Clang Parser  ──→  AST  ──→  Rule Matchers  ──→  Diagnostics
                    or
compile_commands.json
```

### Component Diagram

```
┌─────────────────────────────────────────────────────────┐
│                        CLI (main.cpp)                     │
│  Parses arguments, orchestrates analysis or wrapper mode │
└────────────────┬────────────────────────────────────────┘
                 │
    ┌────────────┼────────────────┐
    │            │                │
    ▼            ▼                ▼
┌────────┐ ┌──────────┐  ┌──────────────┐
│ToolConf│ │CompileDB │  │ BuildWrapper │
│  ig    │ │ Loader   │  │              │
└────────┘ └──────────┘  └──────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│          ClangTool + MatchFinder        │
│  (Clang LibTooling AST traversal)       │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│            RuleRegistry                  │
│  Manages enabled rules, dispatches      │
│  matchers to MatchFinder                │
└────────────────┬────────────────────────┘
                 │
    ┌────────────┼────────────┬────────────┬────────────┐
    ▼            ▼            ▼            ▼            ▼
┌──────┐   ┌──────┐    ┌──────┐    ┌──────┐    ┌──────┐
│CG001 │   │CG002 │    │CG003 │    │CG004 │    │CG005 │
└──┬───┘   └──┬───┘    └──┬───┘    └──┬───┘    └──┬───┘
   │           │           │           │           │
   └───────────┴───────────┴───────────┴───────────┘
                           │
                           ▼
              ┌─────────────────────────┐
              │   DiagnosticReporter     │
              │  Collects, formats,      │
              │  outputs diagnostics     │
              └─────────────────────────┘
```

## Key Design Decisions

### 1. AST Matchers Over RecursiveASTVisitor

We use Clang AST Matchers rather than RecursiveASTVisitor because:
- Matchers provide a declarative, composable API for pattern matching.
- Each rule can independently register its matchers without tight coupling.
- The MatchFinder handles traversal efficiently.

### 2. Rule Abstraction

Each rule implements a common `Rule` interface:
- `id()`: Rule identifier (e.g., "CG001")
- `name()`: Human-readable name
- `description()`: One-line description
- `registerMatchers()`: Register AST matchers with a MatchFinder

Each rule owns one or more `MatchCallback` objects that receive match results and emit diagnostics.

### 3. Two-Phase Design for CG004

The shared memory rule (CG004) requires correlating kernel definitions with kernel launches. We solve this with two matchers in the same pass:
1. First matcher collects kernel functions containing `extern __shared__` variables.
2. Second matcher inspects kernel launches and cross-references the collected set.

This works because Clang processes declarations before their use sites within a translation unit.

### 4. Diagnostic Reporter

The reporter is decoupled from rules:
- Rules create `Diagnostic` objects with structured data.
- The reporter decides output format (human-readable or JSON).
- This separation allows future output formats (SARIF, IDE integrations) without modifying rules.

### 5. Compilation Database Support

Supporting `compile_commands.json` makes CudaGuard work with real build systems. The tool uses:
- `JSONCompilationDatabase::loadFromFile()` for database mode.
- `FixedCompilationDatabase` for single-file mode with extra args.

### 6. Conservative Analysis

All rules are designed to minimize false positives:
- CG001: Only checks literal sibling statements (5 statement lookahead).
- CG002: Only warns on integer literal arguments, not expressions.
- CG003: Skips builtins, implicit functions, and dependent template contexts.
- CG004: Only warns when the third launch parameter is absent or literal zero.
- CG005: Only warns when pointer provenance is high-confidence.

## Build Targets

| Target | Description |
|--------|-------------|
| `cudaguard_core` | Shared library with all rules and infrastructure |
| `cudaguard` | CLI executable |
| `cudaguard_tests` | GoogleTest unit tests |

## Extension Points

Adding a new rule requires:
1. Create header in `include/cudaguard/rules/`
2. Create implementation in `src/rules/`
3. Register in `RuleRegistry::registerAllRules()`
4. Add to CMakeLists.txt source list
5. Add regression test cases
