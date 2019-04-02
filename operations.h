#ifndef VM_OPERATIONS
#define VM_OPERATIONS

#include <stdlib.h>

void add(uint16_t instr);

void bitwise_and(uint16_t instr);

void bitwise_not(uint16_t instr);

void branch(uint16_t instr);

void jump(uint16_t instr);

void jump_to_subroutine(uint16_t instr);

void load(uint16_t instr);

void load_indirect(uint16_t instr);

void load_base_offset(uint16_t instr);

void load_effective_address(uint16_t instr);

void return_from_interrupt(uint16_t instr);

void store(uint16_t instr);

void store_indirect(uint16_t instr);

void store_base_offset(uint16_t instr);

void system_call(uint16_t instr);

#endif
