#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

#include "hardware.h"

uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE* file) {
    // memory location to place the image
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = UINT16_MAX - origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    // swap to little endian
    while (read-- > 0) {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char *image_path) {
    FILE* file = fopen(image_path, "rb");
    if (!file) {
        return 0;
    }
    read_image_file(file);
    fclose(file);
    return 1;
}

uint16_t check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

/*
    Write to memory.
*/
void mem_write(uint16_t addr, uint16_t val) {
    memory[addr] = val;
}

/*
    Read from memory.
*/
uint16_t mem_read(uint16_t addr) {
    if (addr == MR_KBSR) {
        if (check_key()) {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        } else {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[addr];
}

/*
    Input buffering.
*/
struct termios original_tio;

void disable_input_buffering() {
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/*
    Handle interrupt.
*/
void handle_interrupt(int signal) {
    restore_input_buffering();
    printf("\n");
    exit(-2);
}
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
        reg[r_dest] = reg[r_src0] + reg[r_src1];
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
        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg[r_dest] = reg[r_src0] & imm5;
    } else {
        uint16_t r_src1 = instr & 0x7;
        reg[r_dest] = reg[r_src0] & reg[r_src1];
    }
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
    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
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
        putc(c1, stdout);
        char c2 = (*str) >> 8;
        if (c2) putc(c2, stdout);
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
            trap_getc();
            break;
        case TRAP_OUT:
            trap_out();
            break;
        case TRAP_PUTS:
            trap_puts();
            break;
        case TRAP_IN:
            trap_in();
            break;
        case TRAP_PUTSP:
            trap_putsp();
            break;
        case TRAP_HALT:
            trap_halt();
            break;
        default:
            break;
    }
}

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
            case OP_RTI:
            default:
                // bad code
                printf("Unknown operation\n");
                abort();
                break;
        }
    }

    // Shutdown
    restore_input_buffering(); // Unix specific
}
