# Opcodes/Instructions

## Carry Bit Instructions

Format:`0011X111`
`0 = STC, 1 = CMC` \
`CMC` = Complement Carry, inverts carry flag \
`STC` = Set Carry, sets carry = 1

## Single Register Instructions

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