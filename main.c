#include <stdio.h>
#include "hardware.h"
#include "operations.h"
#include "utils.h"

int main(int argc, const char *argv[]) {
    // load arguments
    if (argc < 2) {
        printf("USAGE: lc3 [image-file] ...\n");
        exit(2);
    }

    for (int i = 1; i < argc; ++i) {
        if (!read_image(argv[i])) {
            printf("Failed to load image: %s\n", argv[i]);
            exit(1);
        }
    }

    // Set up
    signal(SIGINT, handle_interrupt);
    disable_input_buffering(); // Unix specific

    // Set PC starting position (default = 0x3000)
    enum { PC_START = 0x3000};
    reg[R_PC] = PC_START;

    while (running) {
        // Fetch
        uint16_t instruction = mem_read(reg[R_PC]++);
        uint16_t op = instruction >> 12; // first 4 bits 

        switch (op)
        {
            case OP_ADD:
                add(instruction);
                break;
            case OP_AND:
                bitwise_and(instruction);
                break;
            case OP_NOT:
                bitwise_not(instruction);
                break;
            case OP_BR:
                branch(instruction);
                break;
            case OP_JMP:
                jump(instruction);
                break;
            case OP_JSR:
                jump_to_subroutine(instruction);
                break;
            case OP_LD:
                load(instruction);
                break;
            case OP_LDI:
                load_indirect(instruction);
                break;
            case OP_LDR:
                load_base_offset(instruction);
                break;
            case OP_LEA:
                load_effective_address(instruction);
                break;
            case OP_ST:
                store(instruction);
                break;
            case OP_STI:
                store_indirect(instruction);
                break;
            case OP_STR:
                store_base_offset(instruction);
                break;
            case OP_TRAP:
                system_call(instruction);
                break;
            case OP_RES:
                abort();
            case OP_RTI:
                return_from_interrupt(instruction); // abort
            default:
                // bad code
                printf("Unknown operation\n");
                break;
        }
    }

    // Shutdown
    restore_input_buffering(); // Unix specific
}
