#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    // REGISTERS
    uint8_t A; // accumulator
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;

    // FLAGS
    bool S; // sign
    bool Z; // zero
    bool AC; // auxillary carry (unused)
    bool P; // parity
    bool CF; // carry

    // STACK POINTER + PROGRAM COUNTER
    uint16_t SP;
    uint16_t PC;

    // MAIN MEMORY
    uint8_t* MEMORY;

} intel8080;

intel8080* cpu_init();
void step(intel8080* state);
void load_program(intel8080* state, const uint8_t* program, size_t size);
uint8_t read_byte(intel8080* state, uint16_t address);
void write_byte(intel8080* state, uint8_t byte, uint16_t address);
void status(intel8080* state);
