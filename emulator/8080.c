#include <stdio.h>
#include "8080.h"

intel8080* cpu_init() {
    intel8080* state = (intel8080*) malloc(sizeof(intel8080));
    state->PC = 0;
    state->SP = 0xFFF; // point in memory to start with
    state->MEMORY = malloc(0x10000);

    return state;
}

void load_program(intel8080* state, const uint8_t* program, size_t size) {
    for(size_t i = 0; i < size; i++) {
        state->MEMORY[i] = program[i];
    }
}

uint8_t read_byte(intel8080* state, uint16_t address) {
    return state->MEMORY[address];
}

void write_byte(intel8080* state, uint8_t byte, uint16_t address) {
    state->MEMORY[address] = byte;
}

void status(intel8080* state) {
    printf("REGISTERS:\n");
    printf("A: 0x%02x\n", state->A);
    printf("B: 0x%02x    | C: 0x%02x\n", state->B, state->C);
    printf("D: 0x%02x    | E: 0x%02x\n", state->D, state->E);
    printf("H: 0x%02x    | L: 0x%02x\n", state->H, state->L);
    printf("PC: 0x%04x | SP: 0x%04x\n", state->PC, state->SP);

    printf("FLAGS:\n");
    printf("S: %d | Z: %d\n", state->S, state->Z);
    printf("P: %d | C: %d\n", state->P, state->CF);
    
    printf("JUST EXECUTED:\n");
    if(state->PC == 0) printf("no instructions yet\n");
    else printf("0x%02x\n", state->MEMORY[state->PC - 1]);

    printf("NEXT INSTRUCTION:\n");
    printf("0x%02x\n\n", state->MEMORY[state->PC]);
}

void step(intel8080* state) {
    // fetch, decode, execute
    uint8_t opcode = read_byte(state, state->PC); // get opcode

    if(opcode == 0) { // NOP
        state->PC += 1;
        return;
    }

    // INSTRUCTION FORMATS
    const uint8_t crbi = 0b00110111; // 0011X111 carry bit
    const uint8_t icsr = 0b00000100; // 00XXX100 increment single reg
    const uint8_t dcsr = 0b00000101; // 00XXX101 decrement single reg
    const uint8_t cmac = 0b00101111; // 00101111 complement accumulator
    const uint8_t move = 0b01000000; // 01XXXXXX mov
    const uint8_t stld = 0b00000010; // 000XX010 stax/ldax
    const uint8_t rmai = 0b10000000; // 10XXXXXX accumulator 
    const uint8_t rtai = 0b00000111; // 000XX111 rotation
    const uint8_t push = 0b11000101; // 11XX0101 register pair
    const uint8_t ppop = 0b11000001; // 11XX0001 register pair
    const uint8_t pdad = 0b00001001; // 00XX1001 double add
    const uint8_t pinx = 0b00000011; // 00XX0011 increment rp
    const uint8_t pdcx = 0b00001011; // 00XX1011 decrement rp
    const uint8_t xchg = 0b11101011; // 11101011 exchange registers
    const uint8_t xthl = 0b11100011; // 11100011 exchange stack
    const uint8_t sphl = 0b11111001; // 11111001 load sp from h/l
    const uint8_t lxip = 0b00000001; // 00XX0001 load immediate rp
    const uint8_t mvip = 0b00000110; // 00XXX110 move immediate data
    const uint8_t imop = 0b11000110; // 11XXX110 immediate operations
    const uint8_t drai = 0b00100010; // 001XX010 direct addressing
    const uint8_t pchl = 0b11101011; // 11101011 H/L into PC
    const uint8_t jump = 0b11000010; // 11XXX01X jump
    const uint8_t call = 0b11000100; // 11XXX10X call
    const uint8_t rfsb = 0b11000000; // 11XXX00X return from subroutine

    // CARRY BIT INSTRUCTIONS
    if(((opcode & 0b11110111) ^ crbi) == 0) {
        // 00110111 == STC, // 00111111 == CMC
        if(opcode & 0b00001000 == 0) state->CF = true; // STC = set carry = 1
        else state->CF = !(state->CF); // CMC = invert carry flag

        state->PC += 1;
        return;
    }
    
    // SINGLE REGISTER INSTRUCTIONS

    // INR = increment register
    if(((opcode & 0b11000111) ^ icsr) == 0 || ((opcode & 0b11000111) ^ dcsr) == 0) {
        // 000 = B, 001 = C, 010 = D, 011 = E, 100 = H, 101 = L, 110 = mem, 111 = A
        // if incrementing
        int add = 1;
        // if decrementing
        if(opcode & 0b00000001 == 1) add = -1;
        
        uint8_t reg = (opcode & 0b00111000) >> 3;
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };
    
        if(reg == 6) state->MEMORY[(state->H << 8) + state->L] += add;
        else *registers[reg] += add;

        state->PC += 1;
        return;
    }
    // DCR = decrement register
    if(((opcode & 0b11000111) ^ dcsr) == 0) {
        // 000 = B, 001 = C, 010 = D, 011 = E, 100 = H, 101 = L, 110 = mem, 111 = A
        uint8_t reg = (opcode & 0b00111000) >> 3;
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };
    
        if(reg == 6) state->MEMORY[(state->H << 8) + state->L] -= 1;
        else *registers[reg] -= 1;

        state->PC += 1;
        return;
    }

    // CMA = complement accumulator
    if(opcode == cmac) {
        state->A = ~state->A;
        
        state->PC += 1;
        return;
    }

    // MOV = move byte from src to dest
    // dest == src == 110 canot happen, 0b01110110 == HLT
    if(((opcode & 0b11000000) ^ move) == 0 && (opcode != 0b01110110)) {
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };

        uint8_t dest = (opcode & 0b00111000) >> 3;
        uint8_t src = (opcode & 0b00000111);

        if(src == 6) *registers[dest] = state->MEMORY[(state->H << 8) + state->L];
        else if(dest == 6) state->MEMORY[(state->H << 8) + state->L] = *registers[src];
        else *registers[dest] = *registers[src];

        state->PC += 1;
        return;
    }
}