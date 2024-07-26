#include <stdio.h>
#include "8080.h"
#include "programs.h"

int main() {
    intel8080* state = cpu_init();
    int size = 12;

    load_program(state, rotation, size);
    status(state);
    for(int i = 0; i < size; i++) {
        step(state);
        status(state);
        getchar();
    }

    free(state->MEMORY);
}