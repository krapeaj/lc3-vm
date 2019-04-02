#include <stdlib.h>
#include <stdio.h>

uint16_t swap(uint16_t x);

void read_image_file(FILE *file);

int read_image(const char *image_path);

void mem_write(uint16_t addr, uint16_t val);

uint16_t mem_read(uint16_t addr);

void disable_input_buffering();

void restore_input_buffering();

void handle_interrupt(int signal);
