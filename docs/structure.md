# Structure

## Registers/Flags (Little Endian)
- A = Accumulator, 8-bit
- B, C, D, E, H, and L, 8-bit 
- Register Pairs:
    - B + C = B, 16-bit
    - D + E = D, 16-bit
    - H + L = H, 16-bit
    - A + F (flags) = PSW, 16-bit
- Program Counter, 16-bit
- Stack Pointer, 16-bit

## Stack Pointer + Subroutines
- Stack Pointer, 16-bit
    - Initialize with `LXI` then push/pop to store/retrieve data
    - During push:
      - 8 MSBs stored at `SP-1`
      - 8 LSBs stored at `SP-2`
      - `SP = SP - 1`
    - During pop:
      - 8 LSBs loaded (from `SP`) into lower register of pair
      - 8 MSBs loaded (from `SP+1`) into higher register of pair
      - `SP = SP - 2`
- Subroutine
  - When `CALL` executed, push address of next instruction on stack then update program counter
  - When `RET` executed, pop stack then update program counter to start execution again

## Status Bits

<style scoped>
table {
    font-size: 20px;
}
</style>

| S | Z | 0 | AC | 0 | P | 1 | C |
|:---:|:---:|:---:|:----:|:---:|:---:|:---:|:---:|
| Sign | Zero | 0 | Auxillary Carry | 0 | Parity | 1 | Carry

- ### Carry Bit
  - For addition **and** subtraction
  - For addition, if result greater than 255, `C = 1`, else `C = 0`
  - For subtraction, carry bit reset of result is positive (bit 7 = 0, 2's complement), else carry bit is set
  - Rotation
    - Left Rotation (`RLC`) & Left through carry (`RAL`)
      - Carry = MSB
    - Right Rotation (`RRC`) & Right through carry (`RAR`)
      - Carry = LSB
- ### Auxillary Carry
  - Auxillary Carry checks bit 3 and it is only used for `DAA`, an instruction to change hex to 2 BCD digits. I am not implementing this instruction because I do not like BCD. So auxillary carry remains unused.
- ### Sign Bit
  - For specific numeric instructions, this gets set if bit 7 of the result (MSB) is equal to 1, otherwise it is reset
- ### Parity
  - This tells you whether there are an odd or even number of 1s in the binary representation of the result. On an actual Intel 8080, this would be calculated with XOR gates like so:
    - Let binary number $b = b_7 b_6 b_5 b_4 b_3 b_2 b_1 b_0$, where each $b_n$ is a digit of the number.
    - Continually XOR them together:
    - $p_3 = b_7 \oplus b_6, p_2 = b_5 \oplus b_4, p_1 = b_3 \oplus b_2, p_0 = b_1 \oplus b_0$.
    - $q_1 = p_3 \oplus p_2, q_0 = p_1 \oplus p_0$.
    - $g = q_1 \oplus q_0$.
    - If $g = 1$, there are an odd number of 1's. If $g = 0$, there are an even number of 1's.
    - The parity flag is the opposite, and if parity = 1 then it is even, and if parity = 0 then it is odd. So `P = !g`.
- ### Zero Bit
  - If the result is equal to zero, this flag is set. Otherwise it is reset.
- ### The other 3 bits
  - Those bits, (the 2nd, 4th, and 6th bits) are permanently 1, 0, and 0 respectively for when we treat Accumulator `A` and the flags as a 16-bit register, collectively known as the `PSW`, or the program status word.