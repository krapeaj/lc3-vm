#ifndef VM_HARDWARE_COMPONENTS
#define VM_HARDWARE_COMPONENTS

#include <stdlib.h>

/* VM running state */
int running = 1;

/* Memory */
uint16_t memory[__UINT16_MAX__];

/* Registers */
typedef enum Register_t {
    R_R0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_COND,
    R_COUNT
} Register;

uint16_t reg[R_COUNT];

/* Memory mapped registers */
typedef enum MMR_t {
    MR_KBSR = 0xFE00, // keyboard status
    MR_KBDR = 0xFE02  // keyboard data
} MMR; 

/* Instruction set */
typedef enum Operation_t {
    OP_BR,  // branch
    OP_ADD, // add
    OP_LD,  // load
    OP_ST,  // store
    OP_JSR, // jump register
    OP_AND, // bitwise and
    OP_LDR, // load register
    OP_STR, // store register
    OP_RTI, // unused
    OP_NOT, // bitwise not
    OP_LDI, // load indirect
    OP_STI, // store indirect
    OP_JMP, // jump
    OP_RES, // reserved (unused)
    OP_LEA, // load effective address
    OP_TRAP // execute trap
} Operation; 

/* Condition flags */
typedef enum ConditionFlag_t{
    FL_POS = 1 << 0, // P
    FL_ZRO = 1 << 1, // Z
    FL_NEG = 1 << 2  // N
} ConditionFlag;

/* TRAP Routines */
typedef enum TrapRoutines_t{
    TRAP_GETC = 0x20, // get character from keyboard
    TRAP_OUT = 0x21, // output a character
    TRAP_PUTS = 0x22, // output a word string
    TRAP_IN = 0x23, // input a string
    TRAP_PUTSP = 0x24, // output a byte string
    TRAP_HALT = 0x25 // halt the program
} TrapRoutine;

#endif
