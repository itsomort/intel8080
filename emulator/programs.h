#include <stdint.h>

// tests STC, CMC
uint8_t carry_bit[2] = {0x37, 0x3F}; 
// increments these registers in order: B, C, D, E, H, L, mem, A
uint8_t increment_reg[8] = {0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C};
// decrements like three registers because i know increment works
uint8_t decrement_reg[3] = {0x15, 0x3d, 0x25};
// increments A then complements it
uint8_t complement[3] = {0x3C, 0x3C, 0x2F};
// increments A, complements it, then moves it into B, H, L, and MEM
uint8_t move[6] = {0x3C, 0x2F, 0x47, 0x67, 0x6F, 0x77};
// increments A, then moves the instruction at mem=0b10000 (increment D) into A
uint8_t stax[3] = {0x3C, 0x14, 0x12};
// increments H twice , then increments memory, then moves H into B, then moves mem into A
uint8_t ldax[5] = {0x24, 0x24, 0x34, 0x44, 0x0A};