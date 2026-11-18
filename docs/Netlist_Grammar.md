# Formal Netlist Grammar & Specification

**Project**: Digital Circuit & Logic Gate Simulator (C++17)  
**Specification**: Object Oriented Programming Project  

---

## 1. Formal Grammar in Extended Backus-Naur Form (EBNF)

```ebnf
netlist         = { line } ;
line            = [ whitespace ] , [ statement ] , [ comment ] , EOL ;

statement       = input_decl
                | output_decl
                | wire_decl
                | gate_decl ;

input_decl      = ( "INPUT"  | "input"  ) , whitespace , ident , { whitespace , ident } ;
output_decl     = ( "OUTPUT" | "output" ) , whitespace , ident , { whitespace , ident } ;
wire_decl       = ( "WIRE"   | "wire"   ) , whitespace , ident , { whitespace , ident } ;

gate_decl       = ( "GATE"   | "gate"   ) , whitespace , 
                  gate_type , whitespace , 
                  ident , whitespace , 
                  ident , { whitespace , ident } , whitespace , 
                  ident ;
                  (* Syntax: GATE <TYPE> <ID> <IN1> [<IN2> ...] <OUT> *)

gate_type       = "AND" | "OR" | "NOT" | "NAND" | "NOR" | "XOR" | "XNOR" ;
                  (* Note: gate type keywords are case-insensitive *)

ident           = ( letter | "_" ) , { letter | digit | "_" } ;
letter          = "A" .. "Z" | "a" .. "z" ;
digit           = "0" .. "9" ;

comment         = "#" , { character - ( "\r" | "\n" ) } ;
whitespace      = { " " | "\t" } ;
EOL             = "\n" | "\r\n" | "\r" ;
```

---

## 2. Semantic Rules & Constraints

1. **Case Sensitivity**:
   - **Keywords**: `INPUT`, `OUTPUT`, `WIRE`, and `GATE` are case-insensitive (e.g., `gate`, `Gate`, and `GATE` are recognized identically).
   - **Gate Types**: `AND`, `OR`, `NOT`, `NAND`, `NOR`, `XOR`, `XNOR` are case-insensitive.
   - **Identifiers**: Wire names and Gate IDs are **case-sensitive** (`A` and `a` are distinct signals).

2. **Order of Declarations**:
   - Wires used in a `GATE` statement (as inputs or output) must be declared in a preceding `INPUT`, `WIRE`, or `OUTPUT` statement.
   - Using an undeclared wire in a gate statement immediately triggers a `NetlistParseException` indicating the offending line number.

3. **Implicit Wire Creation for Outputs**:
   - Signals declared in an `OUTPUT` statement that are not explicitly listed in a `WIRE` statement are automatically registered as valid circuit wires.

4. **Arity Rules**:
   - `NOT` gates require **exactly 1 input** and 1 output wire.
   - `AND`, `OR`, `NAND`, `NOR`, `XOR`, and `XNOR` gates require **at least 2 inputs** and 1 output wire.

5. **Electrical Driving Rules (Topological Validation)**:
   - **Single Driver Constraint**: Every wire can be driven by at most one source (either designated as a primary `INPUT` or driven by exactly one `GATE` output). Any conflict triggers `MultipleDriverException`.
   - **No Floating Inputs**: Every input pin of every gate must have an active driving source. Any floating wire triggers `UnconnectedPinException`.
   - **Driven Outputs**: Every declared primary `OUTPUT` must be driven by a signal source.
   - **Acyclic Combinational Graph**: The circuit dependency graph must be a Directed Acyclic Graph (DAG). Combinational feedback loops (e.g., inverters connected in a circle or unclocked latches) are detected via a 3-color DFS algorithm and trigger `CircuitInstabilityException`.

---

## 3. Valid Netlist Examples

### 3.1 Full Adder (`full_adder.netlist`)
```netlist
# Full Adder Circuit Netlist
# Sum = A ^ B ^ Cin, Cout = (A & B) | (B & Cin) | (A & Cin)

INPUT A B Cin
OUTPUT Sum Cout
WIRE w1 w2 w3
GATE XOR g1 A B w1
GATE XOR g2 w1 Cin Sum
GATE AND g3 A B w2
GATE AND g4 w1 Cin w3
GATE OR g5 w2 w3 Cout
```

### 3.2 2-to-1 Multiplexer (`mux_2to1.netlist`)
```netlist
# 2-to-1 Multiplexer Netlist
# Y = (D0 AND NOT S) OR (D1 AND S)

INPUT D0 D1 S
OUTPUT Y
WIRE notS term0 term1
GATE NOT g_not S notS
GATE AND g_and0 D0 notS term0
GATE AND g_and1 D1 S term1
GATE OR g_or term0 term1 Y
```

---

## 4. Invalid Netlist Test Suite & Error Formats

Each invalid netlist produces an informative, line-numbered diagnostic message formatted as `filename:linenumber: error description`:

| Filename | Triggered Exception | Reported Line | Diagnostic Message Format |
|---|---|:---:|---|
| `err_syntax.netlist` | `NetlistParseException` | Line 4 | `err_syntax.netlist:4: Unknown statement/keyword 'INVALID_KEYWORD'.` |
| `err_unknown_gate.netlist` | `UnknownGateTypeException` | Line 4 | `err_unknown_gate.netlist:4: Unknown gate type 'MAGIC'` |
| `err_undeclared_wire.netlist` | `NetlistParseException` | Line 4 | `err_undeclared_wire.netlist:4: Use of undeclared wire 'undeclared_wire' in GATE 'g1'.` |
| `err_duplicate_wire.netlist` | `NetlistParseException` | Line 4 | `err_duplicate_wire.netlist:4: Duplicate wire declaration 'w1'.` |
| `err_unconnected_pin.netlist` | `UnconnectedPinException` | N/A (Validation) | `Input wire 'floatingWire' of gate 'g1' is floating (has no driver).` |
| `err_multiple_drivers.netlist` | `MultipleDriverException` | N/A (Validation) | `Wire 'Y' has multiple drivers (driven by gate 'g1' and gate 'g2').` |
| `err_feedback_loop.netlist` | `CircuitInstabilityException` | N/A (Validation) | `Combinational feedback loop detected: g1 -> g2 -> g1` |
| `err_no_output.netlist` | `NetlistParseException` | Line 4 | `err_no_output.netlist:4: No OUTPUT declared in netlist file.` |
