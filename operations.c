#include <stdio.h>
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
    Assembler example:
        ADD DR, SR1, SR2
        ADD DR, SR1, imm5
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
    Bit-wise AND operation.
    Assembler example:
        AND DR, SR1, SR2
        AND DR, SR1, imm5
*/
void bitwise_and(uint16_t instr) {
    uint16_t r_dest = (instr >> 9) & 0x7;
    uint16_t r_src0 = (instr >> 6) & 0x7;
    uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        uint16_t imm5 = sign_extend(instr & 0x4, 5);
        reg[r_dest] = reg[r_src0] & imm5;
    } else {
        uint16_t r_src1 = instr & 0x7;
        reg[r_dest] = reg[r_src0] & r_src1;
    }
    update_flags(r_dest);
}

/*
    Conditional Branch (BR)
    BRn LABEL BRzp LABEL
    BRz LABEL BRnp LABEL
*/
void branch(uint16_t instr) {
    uint16_t cond_flag = (instr >> 9) & 0x7;
    if (cond_flag & reg[R_COND]) {
        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
        reg[R_PC] += pc_offset;
    }
}

/*
    Jump (JMP) operation.
    Also handles Return from Subroutine (RET).
*/
void jump(uint16_t instr) {
    uint16_t jump_location = (instr >> 6) & 0x7;
    reg[R_PC] = reg[jump_location];
}

/*
    Jump to Subroutine (JSR and JSRR).
*/
void jump_to_subroutine(uint16_t instr) {
    reg[R_R7] = reg[R_PC];
    uint16_t flag = (instr >> 11) & 0x1;
    if (flag) {
        uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
        reg[R_PC] += pc_offset;
    } else {
        uint16_t r1 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r1];
    }
}

/*
    Load (LD).
*/
void load(uint16_t instr) {
    uint16_t r_dest = (instr >> 9) & 0x7;
    uint16_t pc_offset = (instr & 0x1FF, 9);
    reg[r_dest] = mem_read(reg[R_PC] + pc_offset);
    update_flags(r_dest);
}

/*
    Operation to indirectly load value.
*/
void load_indirect(uint16_t instr) {
    // destination register (DR)
    uint16_t r_dest = (instr >> 9) & 0x7;
    // PC offset (9-bits)
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

    // add pc_offset to the current PC -> another memory address
    // then use that address to load the actual value
    reg[r_dest] = mem_read(mem_read(reg[R_PC] + pc_offset));
    update_flags(r_dest);
}

/*
    Load Base + offset (LDR).
*/
void load_base_offset(uint16_t instr) {
    uint16_t r_dest = (instr >> 9) & 0x7;
    uint16_t r1 = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    reg[r_dest] = mem_read(reg[r1] + offset);
    update_flags(r_dest);
}

/*
    Load Effective Address.
    Assembler format:
        LEA DR, LABEL
*/
void load_effective_address(uint16_t instr) {
    uint16_t r_dest = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    reg[r_dest] = reg[R_PC] + pc_offset;
    update_flags(r_dest);
}

/*
    Bit-wise NOT operation.
*/
void bitwise_not(uint16_t instr) {
    uint16_t r_dest = (instr >> 9) & 0x7;
    uint16_t r_src = (instr >> 6) & 0x7;
    reg[r_dest] = ~reg[r_src];
    update_flags(r_dest);
}

/*
    Return from Interrupt (RTI). UNUSED.
*/
void return_from_interrupt(uint16_t instr) {
    abort();   
}

/*
    Store (ST).
*/
void store(uint16_t instr) {
    uint16_t r_src = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(reg[R_PC] + pc_offset, reg[r_src]);
}

/*
    Store Indirect (STI).
*/
void store_indirect(uint16_t instr) {
    uint16_t r_src = (instr >> 9) & 0x7;
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r_src]);
}

/*
    Store Base + offset (STR).
*/
void store_base_offset(uint16_t instr) {
    uint16_t r_src = (instr >> 9) & 0x7;
    uint16_t r_base = (instr >> 6) & 0x7;
    uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg[r_base] + offset, reg[r_src]);
}

/* GETC system call - get character from keyboard */
void trap_getc() {
    reg[R_R0] = (uint16_t) getchar();
}

/* OUT system call - output a character */
void trap_out() {
    putc((char) reg[R_R0], stdout);
    fflush(stdout);
}

/* PUTS system call - output a string */
void trap_puts() {
    uint16_t *c = memory + reg[R_R0];
    while (*c) {
        putc((char) *c, stdout);
        ++c;
    }
    fflush(stdout);
}

/* IN system call - input a string */
void trap_in() {
    printf("Enter a character: ");
    reg[R_R0] = (uint16_t) getchar();
}

/* PUTSP system call - output a byte string */
void trap_putsp() {
    uint16_t *str = memory + reg[R_R0];
    while (*str) {
        char c1 = (*str) & 0xFF;
        putc((char) c1, stdout);
        char c2 = (*str) >> 8;
        putc((char) c2, stdout);
        ++str;
    }
    fflush(stdout);
}

/* HALT system call - halt the program */
void trap_halt() {
   puts("HALT"); 
   fflush(stdout);
   running = 0;
}

/*
    System Call operation (TRAP).
*/
void system_call(uint16_t instr) {
    switch (instr & 0xFF) {
        case TRAP_GETC:
            /* code */
            break;
        case TRAP_OUT:
            /* code */
            break;
        case TRAP_PUTS:
            /* code */
            break;
        case TRAP_IN:
            /* code */
            break;
        case TRAP_PUTSP:
            /* code */
            break;
        case TRAP_HALT:
            /* code */
            break;
        default:
            break;
    }
}
