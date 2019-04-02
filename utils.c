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
