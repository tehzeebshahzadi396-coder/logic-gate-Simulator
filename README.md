# Digital Circuit & Logic Gate Simulator (C++17)

A modular, extensible digital logic simulator implemented in modern C++17 as an Object Oriented Programming project.

---

## Overview

The Digital Circuit & Logic Gate Simulator is a simulation engine that parses text-based netlist descriptions (`.netlist`), constructs in-memory directed circuit graphs, detects combinational cycles, validates electrical driving constraints, and simulates multi-pass signal propagation to convergence. It supports exhaustive truth-table generation across $2^N$ input combinations, formatted ASCII tables, and CSV exports.

---

## Key Features

1. **Modern C++17 Architecture**:
   - Zero raw owning pointers (`std::shared_ptr` / `std::unique_ptr` used throughout).
   - Strict const-correctness, `[[nodiscard]]`, `override`, `final`, `explicit`, and `#pragma once`.
   - Kleene 3-valued logic system supporting `LOW (0)`, `HIGH (1)`, and `UNKNOWN (X)` signal states.
2. **Object-Oriented Design**:
   - Comprehensive demonstration of Encapsulation, Abstraction, Inheritance, Polymorphism, Composition, and Custom Exception Handling.
   - Template Method pattern for gate evaluation.
   - Extensible runtime `GateFactory` registry.
3. **Robust Circuit Validation**:
   - Three-color (WHITE/GRAY/BLACK) DFS cycle detection identifying exact combinational feedback loops.
   - Strict pin connection rules (floating input detection and multiple-driver conflict rejection).
4. **Multi-Pass Convergence Simulation**:
   - Dynamic signal propagation converging to a stable state across arbitrary gate declaration orders.
5. **Truth Table & Export Engine**:
   - Exhaustive $2^N$ binary enumeration (up to $N=20$) with column-aligned ASCII truth tables and CSV file export.
6. **Command-Line & Interactive Menu Interface**:
   - Command-line arguments (`--table`, `--set`, `--csv`, `--verbose`, `--help`) alongside an interactive console UI.

---

## OOP Concept Mapping

| OOP Concept | Implementing Classes / Files | Concrete Demonstration |
|---|---|---|
| **Encapsulation** | [`Wire.h`](include/Wire.h), [`Wire.cpp`](src/Wire.cpp) | Private attributes (`m_state`, `m_hasDriver`, `m_isPrimaryInput`, `m_isPrimaryOutput`); mutation through `setState()` returning boolean dirty-flag. |
| **Abstraction** | [`Gate.h`](include/Gate.h), [`Gate.cpp`](src/Gate.cpp) | Abstract base class defining `compute() const = 0` customization point and `evaluate()` Template Method pattern. |
| **Inheritance** | [`Gates.h`](include/Gates.h), [`Gates.cpp`](src/Gates.cpp), [`Exceptions.h`](include/Exceptions.h) | Derived gates (`ANDGate`, `ORGate`, `NOTGate`, etc.) inherit from `Gate`. Domain exceptions inherit from `SimulatorException`. |
| **Polymorphism** | [`Circuit.h`](include/Circuit.h), [`Circuit.cpp`](src/Circuit.cpp) | Gates stored and simulated via `std::vector<std::shared_ptr<Gate>>` using virtual method dispatch without runtime branching. |
| **Composition** | [`Circuit.h`](include/Circuit.h), [`Circuit.cpp`](src/Circuit.cpp) | `Circuit` container owns lifetime of all `Wire` and `Gate` graph components via smart pointers; copy disabled, move enabled. |
| **Custom Exceptions** | [`Exceptions.h`](include/Exceptions.h), [`NetlistParser.cpp`](src/NetlistParser.cpp) | Line-numbered parse exceptions (`NetlistParseException`), cycle exceptions (`CircuitInstabilityException`), driver conflicts (`MultipleDriverException`). |

---

## Project Structure

```
logic_simulator/
├── include/
│   ├── LogicState.h            # 3-valued Kleene logic enum & free functions
│   ├── Wire.h                  # Encapsulated digital signal wire
│   ├── Gate.h                  # Abstract base gate (Template Method)
│   ├── Gates.h                 # Concrete gates & extensible GateFactory registry
│   ├── Circuit.h               # Circuit composition container & simulation engine
│   ├── NetlistParser.h         # EBNF grammar netlist reader & semantic validator
│   ├── Exceptions.h            # Comprehensive custom exception hierarchy
│   └── TruthTableGenerator.h   # 2^N combination generator & ASCII/CSV exporter
├── src/
│   ├── Wire.cpp
│   ├── Gate.cpp
│   ├── Gates.cpp
│   ├── Circuit.cpp
│   ├── NetlistParser.cpp
│   ├── TruthTableGenerator.cpp
│   └── main.cpp                # CLI parser, interactive menu, and error dispatch
├── netlists/
│   ├── half_adder.netlist      # 1-bit Half Adder netlist
│   ├── full_adder.netlist      # 1-bit Full Adder netlist
│   ├── mux_2to1.netlist        # 2-to-1 Multiplexer netlist
│   ├── carry_adder_4bit.netlist# 4-bit Ripple Carry Adder netlist
│   └── invalid/                # 8 broken netlist test cases for exception verification
├── tests/
│   └── test_main.cpp           # Self-contained comprehensive unit & integration test runner
├── docs/
│   ├── UML_ClassDiagram.md     # Mermaid diagram and architecture breakdown
│   ├── Netlist_Grammar.md      # Formal EBNF grammar and error rules
│   └── TruthTable_Verification.md # Truth tables and formal Boolean algebra proofs
├── CMakeLists.txt              # CMake build configuration
├── Makefile                    # Standalone GNU Makefile
└── README.md                   # This documentation
```

---

## Build and Run Instructions

The project requires a C++17 compliant compiler (`g++`, `clang++`, or MSVC).

### Option A: Using GNU Make (Windows MinGW / Linux / macOS)
```bash
# Build executables (logic_sim and run_tests)
make all

# Run test suite
make test

# Clean build artifacts
make clean
```

*(On Windows with MinGW, use `mingw32-make` if `make` is not aliased)*.

### Option B: Using CMake
```bash
# Configure build directory
cmake -B build -G "MinGW Makefiles"    # or "Unix Makefiles" on Linux/macOS

# Build all targets
cmake --build build

# Execute test suite via CTest
ctest --test-dir build --output-on-failure
```

---

## Command-Line Usage

```bash
./logic_sim <file.netlist> [options]
```

### Options:
- `--table` : Generate and display truth table (default behavior).
- `--set <A=1,B=0,...>` : Simulate a single custom input vector and print outputs.
- `--verbose` : Display pass-by-pass multi-pass signal propagation.
- `--csv <file.csv>` : Export truth table to a CSV file.
- `--help, -h` : Display usage instructions.

### Examples:

1. **Generate Truth Table for Full Adder**:
   ```bash
   ./logic_sim netlists/full_adder.netlist --table
   ```
   *Output*:
   ```
   | A | B | Cin || Sum | Cout |
   |---|---|-----||-----|------|
   | 0 | 0 |  0  ||  0  |  0   |
   | 0 | 0 |  1  ||  1  |  0   |
   | 0 | 1 |  0  ||  1  |  0   |
   | 0 | 1 |  1  ||  0  |  1   |
   | 1 | 0 |  0  ||  1  |  0   |
   | 1 | 0 |  1  ||  0  |  1   |
   | 1 | 1 |  0  ||  0  |  1   |
   | 1 | 1 |  1  ||  1  |  1   |
   ```

2. **Simulate a Single Input Vector with Multi-pass Logging**:
   ```bash
   ./logic_sim netlists/full_adder.netlist --set A=1,B=1,Cin=1 --verbose
   ```
   *Output*:
   ```
   [Pass 1]
     Gate 'g1' (XOR) updated wire 'w1': X -> 0
     Gate 'g2' (XOR) updated wire 'Sum': X -> 1
     Gate 'g3' (AND) updated wire 'w2': X -> 1
     Gate 'g4' (AND) updated wire 'w3': X -> 0
     Gate 'g5' (OR) updated wire 'Cout': X -> 1
   [Pass 2]
   Circuit converged in 2 pass(es).
   Simulation result for [A=1,B=1,Cin=1]:
     Sum = 1
     Cout = 1
   Settled in 2 pass(es).
   ```

3. **Export Truth Table to CSV**:
   ```bash
   ./logic_sim netlists/mux_2to1.netlist --csv mux_out.csv
   ```

4. **Launch Interactive Menu**:
   ```bash
   ./logic_sim
   ```

---

## Test Suite Coverage

The automated test runner (`run_tests`) executes 10 test suites covering:
1. **LogicState & Kleene 3-Valued Algebra**: Complete truth tables for `NOT`, `AND`, `OR`, `XOR`, including dominance laws (`LOW AND X = LOW`, `HIGH OR X = HIGH`, `XOR with X = X`).
2. **Wire Encapsulation**: Default states, dirty-flag detection on state changes, signal resetting.
3. **Gate Arity & Functional Bounds**: 2-input and 3-input truth tables for `AND`, `OR`, `NAND`, `NOR`, `XOR`, `XNOR`, and `NOT`.
4. **Polymorphic Dispatch**: Evaluation of diverse gate types through base `Gate*` pointers.
5. **Half Adder Simulation**: All 4 states verified.
6. **Full Adder Arithmetic Verification**: All 8 states verified against binary arithmetic addition.
7. **2:1 Multiplexer Verification**: All 8 states verified against mux switching logic.
8. **4-Bit Ripple Carry Adder Verification**: Exhaustive verification of all 512 input vectors ($A \in [0..15], B \in [0..15], \text{Cin} \in \{0, 1\}$) against integer addition.
9. **Multi-Pass Reverse Settling**: Gates declared in reverse topological order settle accurately over multiple passes ($>1$).
10. **Error Suite**: All 8 invalid netlists trigger the appropriate custom exceptions with accurate line numbers and diagnostic messages.
