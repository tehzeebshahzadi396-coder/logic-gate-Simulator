# Truth Table Verification & Boolean Algebra Proofs

**Project**: Digital Circuit & Logic Gate Simulator (C++17)  
**Specification**: Object Oriented Programming Project  

---

## 1. Half Adder Verification

### Circuit Equations
$$\text{Sum} = A \oplus B$$
$$\text{Cout} = A \cdot B$$

### Simulator Output
```
| A | B || Sum | Cout |
|---|---||-----|------|
| 0 | 0 ||  0  |  0   |
| 0 | 1 ||  1  |  0   |
| 1 | 0 ||  1  |  0   |
| 1 | 1 ||  0  |  1   |
```

### Analytical Verification
1. $A=0, B=0 \implies \text{Sum} = 0 \oplus 0 = 0,\; \text{Cout} = 0 \cdot 0 = 0$ (Match)
2. $A=0, B=1 \implies \text{Sum} = 0 \oplus 1 = 1,\; \text{Cout} = 0 \cdot 1 = 0$ (Match)
3. $A=1, B=0 \implies \text{Sum} = 1 \oplus 0 = 1,\; \text{Cout} = 1 \cdot 0 = 0$ (Match)
4. $A=1, B=1 \implies \text{Sum} = 1 \oplus 1 = 0,\; \text{Cout} = 1 \cdot 1 = 1$ (Match)

---

## 2. Full Adder Verification

### Circuit Equations
$$\text{Sum} = A \oplus B \oplus \text{Cin}$$
$$\text{Cout} = (A \cdot B) + (\text{Cin} \cdot (A \oplus B)) = (A \cdot B) + (B \cdot \text{Cin}) + (A \cdot \text{Cin})$$

### Simulator Output
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

### Analytical Verification
| A | B | Cin | Decimal Sum ($A+B+\text{Cin}$) | Binary Result ($\text{Cout}\;\text{Sum}$) | Simulated Sum | Simulated Cout | Verification |
|:-:|:-:|:---:|:---:|:---:|:---:|:---:|:---:|
| 0 | 0 | 0 | 0 | `00` | 0 | 0 | **PASS** |
| 0 | 0 | 1 | 1 | `01` | 1 | 0 | **PASS** |
| 0 | 1 | 0 | 1 | `01` | 1 | 0 | **PASS** |
| 0 | 1 | 1 | 2 | `10` | 0 | 1 | **PASS** |
| 1 | 0 | 0 | 1 | `01` | 1 | 0 | **PASS** |
| 1 | 0 | 1 | 2 | `10` | 0 | 1 | **PASS** |
| 1 | 1 | 0 | 2 | `10` | 0 | 1 | **PASS** |
| 1 | 1 | 1 | 3 | `11` | 1 | 1 | **PASS** |

All 8 rows match exact arithmetic binary addition.

---

## 3. 2-to-1 Multiplexer Verification

### Circuit Equation
$$Y = (D_0 \cdot \overline{S}) + (D_1 \cdot S)$$

### Simulator Output
```
| D0 | D1 | S || Y |
|----|----|---||---|
| 0  | 0  | 0 || 0 |
| 0  | 0  | 1 || 0 |
| 0  | 1  | 0 || 0 |
| 0  | 1  | 1 || 1 |
| 1  | 0  | 0 || 1 |
| 1  | 0  | 1 || 0 |
| 1  | 1  | 0 || 1 |
| 1  | 1  | 1 || 1 |
```

### Analytical Verification
- When $S = 0$, $Y = (D_0 \cdot 1) + (D_1 \cdot 0) = D_0$:
  - $D_0=0, D_1=0 \implies Y = 0$
  - $D_0=0, D_1=1 \implies Y = 0$
  - $D_0=1, D_1=0 \implies Y = 1$
  - $D_0=1, D_1=1 \implies Y = 1$
- When $S = 1$, $Y = (D_0 \cdot 0) + (D_1 \cdot 1) = D_1$:
  - $D_0=0, D_1=0 \implies Y = 0$
  - $D_0=0, D_1=1 \implies Y = 1$
  - $D_0=1, D_1=0 \implies Y = 0$
  - $D_0=1, D_1=1 \implies Y = 1$

All 8 states strictly conform to the 2:1 multiplexer specification.

---

## 4. 4-Bit Ripple Carry Adder Verification

### Circuit Structure
The 4-bit adder cascades four full-adder stages ($i \in \{0, 1, 2, 3\}$):
$$S_i = A_i \oplus B_i \oplus C_i$$
$$C_{i+1} = (A_i \cdot B_i) + (C_i \cdot (A_i \oplus B_i))$$
where $C_0 = \text{Cin}$ and $C_4 = \text{Cout}$.

### Exhaustive Verification
- Total inputs: 9 primary inputs ($A_0..A_3, B_0..B_3, \text{Cin}$).
- Total combinations: $2^9 = 512$ combinations ($16 \times 16 \times 2$).
- Verified by: `test4BitAdderExhaustive512()` in `tests/test_main.cpp`.
- Expected invariant:
$$\sum_{i=0}^3 S_i \cdot 2^i + \text{Cout} \cdot 2^4 = \left(\sum_{i=0}^3 A_i \cdot 2^i\right) + \left(\sum_{i=0}^3 B_i \cdot 2^i\right) + \text{Cin}$$

**Result**: 512 out of 512 test vectors passed with 100% precision.
