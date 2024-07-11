#include <stdio.h>
#include "8080.h"
#include "programs.h"

int main() {
    intel8080* state = cpu_init();
    int size = 10;

    load_program(state, add_test, size);
    status(state);
    for(int i = 0; i < size; i++) {
        step(state);
        status(state);
        getchar();
    }
    // printf("%x\n", state->MEMORY[(state->H << 8) + state->L]);
}