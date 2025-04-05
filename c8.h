#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#include <constants.h>
#include <sdl.h>

typedef struct {
    uint8_t mem[CHIP8_MEMORY_SPACE];
    uint8_t regs[CHIP8_REGISTERS_COUNT];
    uint8_t dt;
    uint8_t st;
    uint16_t i;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[CHIP8_STACK_SIZE];   
    sdl_t *sdl;
    bool display[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    bool keys[16];
    uint8_t key;
} chip8_t;

typedef struct {
    void *content;
    size_t size;
} chip8_program_t;

chip8_t chip8_init(sdl_t *sdl, chip8_program_t program);
void chip8_renderdisplay(chip8_t *c);
void chip8_exec(chip8_t *c);

#endif