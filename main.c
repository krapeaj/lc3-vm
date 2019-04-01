#include "hardware.h"
#include "operations.h"

int main(int argc, const *argv[]) {

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
}
