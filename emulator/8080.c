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

void parity8(intel8080* state, uint8_t result) {
    // typically, you would grab individual bits and XOR them
    // to get the parity, but getting individual bits in C
    // is harder than doing it with logic gates,
    // so the comfort of regular programming languages comes to the rescue

    bool even = true;
    for(int i = 0; i < 8; i++) {
        uint8_t a = result & (1 << i);
        if(a != 0) even = !even;
    }

    state->P = even;
}

void parity16(intel8080* state, uint16_t result) {
    // same as parity8 except i < 16

    bool even = true;
    for(int i = 0; i < 16; i++) {
        uint16_t a = result & (1 << i);
        if(a != 0) even = !even;
    }

    state->P = even;
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
    uint8_t opcode = state->MEMORY[state->PC]; // get opcode

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
        if((opcode & 0b00001000) == 0) state->CF = true; // STC = set carry = 1
        else state->CF = !(state->CF); // CMC = invert carry flag

        goto increment;
    }
    
    // SINGLE REGISTER INSTRUCTIONS

    // INR = increment register
    // DCR = decrement register
    if(((opcode & 0b11000111) ^ icsr) == 0 || ((opcode & 0b11000111) ^ dcsr) == 0) {
        // 000 = B, 001 = C, 010 = D, 011 = E, 100 = H, 101 = L, 110 = mem, 111 = A
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };
        
        // if incrementing
        int add = 1;
        // if decrementing
        if((opcode & 0b00000001) == 1) add = -1;
        
        uint8_t reg = (opcode & 0b00111000) >> 3;
        
        // affects zero, sign, parity
    
        uint8_t result;

        if(reg == 6) {
            state->MEMORY[(state->H << 8) + state->L] += add;
            result = state->MEMORY[(state->H << 8) + state->L];
        }
        else {
            registers[reg] += add;
            result = *registers[reg];
        }

        state->Z = result == 0;
        state->S = (result >> 7);
        parity8(state, result);

        goto increment;
    }

    // CMA = complement accumulator
    if(opcode == cmac) {
        state->A = ~state->A;
        
        goto increment;
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
        else if(dest == 6) state->MEMORY[(state->H << 8) + state->L] = *(registers[src]);
        else *(registers[dest]) = *(registers[src]);

        goto increment;
    }

    // STAX = accumulator -> memory pointed to by B/C or D/E
    // LDAX = memory pointed to by B/C or D/E -> accumulator
    if(((opcode & 0b11100111) ^ stld) == 0) {
        // get memory location
        // 0000X010 = B/C, 0001X010 = D/E
        uint16_t mem = 0;
        if((opcode & 0b00010000) == 0) mem = (state->B << 8) + state->C;
        else mem = (state->D << 8) + state->E;

        // stax = 000X0010, ldax = 000X1XXX
        if((opcode & 0b00001000) == 0) state->MEMORY[mem] = state->A; // stax
        else state->A = state->MEMORY[mem]; // ldax

        goto increment;
    }

    // REGISTER/MEMORY ACCUMULATOR INSTRUCTIONS
    if(((opcode & 0b11000000) ^ rmai) == 0) {
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };

        uint8_t reg = (opcode & 0b00000111);
        uint8_t value;
        if(reg == 6) value = state->MEMORY[((state->H) << 8) + state->L];
        else value = *registers[reg]; 

        uint8_t operation = (opcode & 0b00111000) >> 3;
        uint16_t result;

        switch(operation) {
            case 0: // ADD
                result = state->A + value;
                state->A = (uint8_t) result;
                state->CF = (result > 255);
                state->Z = (state->A == 0);
                parity8(state, state->A);
            break;

            case 1: // ADC
                result = state->A + value + state->CF;
                state->A = (uint8_t) result;
                state->CF = (result > 255);
                state->Z = (state->A == 0);
                parity8(state, state->A);
            break;

            case 2: // SUB
                // uint8_t comp = (~value) + 1;
                // result = (uint8_t) state->A + comp;
                // state->CF = !(result > 255);
                // state->Z = (state->A == 0);
                // state->S = (state->A >> 7);
                // parity8(state, state->A);
            break;

            case 3: // SBB
            break;

            case 4: // ANA
            break;

            case 5: // XRA
            break;

            case 6: // ORA
            break;

            case 7: // CMP
            break;

            default:
                printf("somehow got an operation that is not 1-7");
        }
        
        goto increment;
    }


    // LXI - Load Immediate Register Pair

    if(((opcode & 0b11001111) ^ lxip) == 0) {
        uint8_t reg = (opcode & 0b00110000) >> 4;
        if(reg == 0) { // reg = B/C
            state->C = state->MEMORY[state->PC + 1];
            state->B = state->MEMORY[state->PC + 2];
        }
        else if(reg == 1) { // reg = D/E
            state->E = state->MEMORY[state->PC + 1];
            state->D = state->MEMORY[state->PC + 2];
        }
        else if(reg == 2) { // reg = H/L
            state->L = state->MEMORY[state->PC + 1];
            state->H = state->MEMORY[state->PC + 2];
        }
        else if(reg == 3) { // reg = SP
            uint8_t dataL = state->MEMORY[state->PC + 1];
            uint8_t dataH = state->MEMORY[state->PC + 2];
            state->SP = (dataH << 8) + dataL;
        }
        
        state->PC += 2;
        goto increment;
    }

    // MVI - Move Immediate Data

    if(((opcode & 0b11000111) ^ mvip) == 0) {
        uint8_t *registers[8] = {
            &state->B, &state->C,
            &state->D, &state->E,
            &state->H, &state->L,
            NULL,      &state->A
        };

        uint8_t reg = (opcode & 0b00111000) >> 3;
        uint8_t data = state->MEMORY[state->PC + 1];

        if(reg == 6) state->MEMORY[(state->H << 8) + (state->L)] = data;
        else *(registers[reg]) = data;

        state->PC += 1;
        goto increment;
    }

    increment:
    state->PC += 1;
    return;
}