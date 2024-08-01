# Opcodes/Instructions

## Instruction Types (for assembler)

1 = carry bit\
2 = single register (inr, drc, cma)\
3 = NOP\
4 = MOV\
5 = LDAX/STAX\
6 = reg/mem accumulator arithmetic\
7 = LXI\
8 = MVI\
9 = immediate arithmetic\
10 = rotation\
11 = push/pop\
12 = direct addressing\
13 = jumps\
14 = call\
15 = ret\
16 = register pair 1 (begins with 00)\
17 = register pair 2 (begins with 111)\


## Carry Bit Instructions

Format:`0011X111`
`0 = STC, 1 = CMC` \
`CMC` = Complement Carry, inverts carry flag \
`STC` = Set Carry, sets carry = 1

## Single Register Instructions

| Register | Code |
|:-:|:-:|
| B | 000 |
| C | 001 |
| D | 010 |
| E | 011 |
| H | 100 |
| L | 101 |
| M*| 110 |
| A | 111 |

M\*: Address in memory pointed to by the H and L registers

`INR`: Increment Register/Memory \
`00XXX100` `XXX = Register` \
Increments register/memory by 1. \
Affects: Z, S, P, AC

`DCR`: Decrement Register/Memory \
`00XXX101` `XXX = Register` \
Decrements register/memory by 1. \
Affects: Z, S, P, AC

`CMA`: Complement Accumulator \
`00101111` \
Complements the accumulator (register A).

`DAA`: Not implementing

## NOP Instruction

`NOP`: No operation, just stalls \
`00000000` 

Just as a note, doing `MOV` between two of the same registers is technically another form of `NOP`.

## Data Transfer Instructions

### `MOV`

| Register | Code |
|:-:|:-:|
| B | 000 |
| C | 100 |
| D | 010 |
| E | 011 |
| H | 100 |
| L | 101 |
| M*| 110 |
| A | 111 |

M\*: Address in memory pointed to by the H and L registers

`MOV`: Move between registers \
`01 DST SRC`: `DST` = Destination Register, `SRC` = Source Register \
Note: Destination and Source cannot both be `110`. \
Moves one byte from Source to Destination. Source remains unchanged.

### `LDAX`/`STAX`

`000XY010`: `X: 0 =` Reg Pair B, `1 =` Reg Pair D | `Y: 0 =` STAX, `1 =` LDAX

`STAX`: Contents of accumulator stored in memory pointed to by Register Pair B or Register Pair D \
`LDAX`: Contents of memory pointed to by Register Pair B or Register Pair D loaded into accumulator

For both `STAX` and `LDAX`, B/D are the upper bits and C/E are the lower bits. EX: `B=0x13`, `C=0xEA`, Address = `$13EA` not `$EA13`.

## Register/Memory Accumulator Instructions

Format: `10 XXX YYY` \
`XXX =` Operation \
`YYY =` Register

| Operation | Code | Register | Code |
|:-:|:-:|:-:|:-:|
|`ADD`| 000 | B  | 000 |
|`ADC`| 001 | C  | 001 |
|`SUB`| 010 | D  | 010 |
|`SBB`| 011 | E  | 011 |
|`ANA`| 100 | H  | 100 |
|`XRA`| 101 | L  | 101 |
|`ORA`| 110 | M* | 110 |
|`CMP`| 111 | A  | 111 |

M\*: Address in memory pointed to by the H and L registers

### Operations:
- `ADD`
  - Add value from register/memory to accumulator
  - Affects: C, S, Z, P, AC
- `ADC`
  - Add value from register/memory to accumulator **PLUS** carry
  - Affects: C, S, Z, P, AC
- `SUB`
  - Subtract register/memory from accumulator using two's complement
  - Affects: C, S, Z, P, AC
- `SBB`
  - Subtract register/memory from accumulator with borrow
  - Add carry to register/memory, then subtract from accumulator
  - Affects: C, S, Z, P, AC
- `ANA`:
  - Logical AND with A, reset carry
  - Affects: C, S, Z, P
- `XRA`
  - Logical XOR with A, reset carry
  - Affects: C, S, Z, P
- `ORA`
  - Logical OR with A, reset carry
  - Affects: C, S, Z, P
- `CMP`
  - Compare register/memory with A with subtraction
  - If A - value = 0, then reg = A
  - Carry will be set if no carry out meaning reg > A
  -  | REG vs A | Z | C | 
     |:-:|:-:|:-:|
     | REG = A | 1 | 0 |
     | REG > A | 0 | 1 |
     | REG < A | 0 | 0 |

## Immediate Instructions

### LXI - Load Immediate Register Pair (not entirely sure on the name...)

Format: `00XX0001 DAT1 DAT2`, each `DATA = 8 bits` \
`XX` = Chosen register pair \
`DAT1` = Lower Bits \
`DAT2` = Higher Bits
|Register Pair|Code|
|:-:|:-:|
|B/C|00|
|D/E|01|
|H/L|10|
|SP|11|

Little endian, first operand (8 bits) moved into second register of pair, second operand (8 bits) moved into first register of pair. In case of stack pointer, load first data into lower bytes, load second data into higher bytes.

### MVI - Move Immediate Data

Format: `00XXX110 DATA`, `DATA = 8 bits` \
`XXX` = Chosen Register 
| Register | Code |
|:-:|:-:|
| B | 000 |
| C | 100 |
| D | 010 |
| E | 011 |
| H | 100 |
| L | 101 |
| M*| 110 |
| A | 111 |

M\*: Address in memory pointed to by the H and L registers

8 bits of data get loaded into the chosen register or memory in the case of M*.

### Immediate Arithmetic Instructions

Format: `11XXX110 DATA`, `XXX` = Operation, `DATA = 8 bits` \
Note: All instructions function on accumulator.

| Operation | Code | Equivalent
|:-:|:-:|:-:|
| `ADI` | 000 | `ADD` |
| `ACI` | 001 | `ADC` |
| `SUI` | 010 | `SUB` |
| `SBI` | 011 | `SBB` |
| `ANI` | 100 | `ANA` |
| `XRI` | 101 | `XRA` |
| `ORI` | 110 | `ORA` |
| `CPI` | 111 | `CMP` |

These operations function the exact same as the non-immediate versions (`ADD`, `ADC`, etc.) so their behavior will not be gone into detail here. For details, look at the equivalent operation in the Register/Memory Accumulator Instructions section.

## Rotation Instructions
Format: `000XX111`, `XX` = Operation

| Operation | Mnemonic | Code
|:-:|:-:|:-:|
Rotate Accumulator Left | RLC | 00
Rotate Accumulator Right | RRC | 01
Rotate Accumulator Left Through Carry | RAL | 10
Rotate Accumulator Right Through Carry | RAR | 11

- ### RLC:
  - Rotate Accumulator Left, carry bit = MSB
  - High order bit transferred to LSB
- ### RRC:
  - Rotate Accumulator Right, carry bit = LSB
  - Low order bit transferred to MSB
- ### RAL:
  - Rotate Left Through Carry, Carry -> Temp, MSB -> Carry, rotate left, Temp -> LSB
- ### RAR:
  - Rotate Right Through Carry, Carry -> Temp, LSB -> Carry, rotate right, Temp -> MSB