#include "hardware.h"
#include "operations.h"

int main(int argc, const *argv[]) {

    // Set PC starting position (default = 0x3000)
    enum { PC_START = 0x3000};
    reg[R_PC] = PC_START;

    int running = 1;
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
                /* code */
                break;
            case OP_NOT:
                /* code */
                break;
            case OP_BR:
                /* code */
                break;
            case OP_JMP:
                /* code */
                break;
            case OP_JSR:
                /* code */
                break;
            case OP_LD:
                /* code */
                break;
            case OP_LDI:
                /* code */
                break;
            case OP_LDR:
                /* code */
                break;
            case OP_LEA:
                /* code */
                break;
            case OP_ST:
                /* code */
                break;
            case OP_STI:
                /* code */
                break;
            case OP_STR:
                /* code */
                break;
            case OP_TRAP:
                /* code */
                break;
            case OP_RES:
            case OP_RTI:
            default:
                // bad code
                break;
        }
    }
}
