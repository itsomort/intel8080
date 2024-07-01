#include <stdio.h>
#include "8080.h"
#include "programs.h"

int main() {
    intel8080* state = cpu_init();
    int size = 5;

    load_program(state, ldax, size);
    status(state);
    for(int i = 0; i < size; i++) {
        step(state);
        status(state);
    }
    printf("%x\n", state->MEMORY[(state->B << 8) + state->C]);
}