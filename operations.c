#include "hardware.h"

/*
    To extend a number to be 16 bits, whether the number is
    positive or negative.
*/
uint16_t sign_extend(uint16_t n, int bit_count) {
    if ((n >> (bit_count - 1)) & 1) {
        n |= (0xFFFF << bit_count);
    }
    return n;
}

/*
    Any time a value is written to a register, we need to update the flags
    to indicate its sign.
*/
void update_flags(uint16_t r) {
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) {
        reg[R_COND] = FL_NEG;
    }
    else {
        reg[R_COND] = FL_POS;
    }
}

/*
    ADD operation.
*/
void add(uint16_t instr) {
    // destination register (DR)
    uint16_t r_dest = (instr >> 9) & 0x7;
    // first operand
    uint16_t r_src0 = (instr >> 6) & 0x7;
    // immediate mode flag
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r_dest] = reg[r_src0] + imm5;
    }
    else {
        uint16_t r_src1 = instr & 0x7;
        reg[r_dest] = reg[r_src0] + r_src1;
    }
    update_flags(r_dest);
}

/*
    Operation to indirectly load value.
*/
void load_indirect(u_int16_t instr) {
    // destination register (DR)
    uint16_t r_dest = (instr >> 9) & 0x7;
    // PC offset (9-bits)
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    // add pc_offset to the current PC -> another memory address
    // then use that address to load the actual value
    reg[r_dest] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r_dest);
}
