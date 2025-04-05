#include <c8.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

void chip8_loadprogram(chip8_t *c, chip8_program_t program);
void chip8_loadfont(chip8_t *c);

chip8_t chip8_init(sdl_t *sdl, chip8_program_t program) {
    chip8_t c = (chip8_t) {0};
    c.sdl = sdl;
    c.pc = CHIP8_PROGRAMS_OFFSET;
    chip8_loadfont(&c);
    chip8_loadprogram(&c, program);
    return c;
}

void chip8_loadprogram(chip8_t *c, chip8_program_t program) {
    memcpy(c->mem + CHIP8_PROGRAMS_OFFSET, program.content, program.size);
}

void chip8_loadfont(chip8_t *c) {
    #include <font.c>
    memcpy(c->mem, fonts, sizeof(uint8_t) * FONT_ARRAY_LEN);
}

uint16_t chip8_fetch(chip8_t *c) {
    uint8_t most = c->mem[c->pc];
    uint8_t least = c->mem[c->pc + 1];
    return (uint16_t)((most << 8) + least);
}

uint16_t chip8_pop(chip8_t *c) {
    assert(c->sp > 0 && "stack underflow");
    return c->stack[(size_t)(--c->sp)];
}

void chip8_push(chip8_t *c, uint16_t v) {
    assert(c->sp == CHIP8_STACK_SIZE && "stack overflow");
    c->stack[(size_t)(c->sp++)] = v;
}

void chip8_cleardisplay(chip8_t *c) {
    for(size_t row = 0; row < DISPLAY_HEIGHT; ++row) {
        for(size_t col = 0; col < DISPLAY_WIDTH; ++col) {
            c->display[row][col] = 0;
        }
    }
}

void chip8_clearkeys(chip8_t *c) {
    for(size_t i = 0; i < 16; i++) {
        c->keys[i] = false;
    }
}

void chip8_readkey(chip8_t *c) {
    chip8_clearkeys(c);

    while(true) {
        SDL_KeyCode key = sdl_readkey();
        switch (key) {
            case SDLK_1: c->key = 0x1; return;
            case SDLK_2: c->key = 0x2; return;
            case SDLK_3: c->key = 0x3; return;
            case SDLK_4: c->key = 0xC; return;
            case SDLK_q: c->key = 0x4; return;
            case SDLK_w: c->key = 0x5; return;
            case SDLK_e: c->key = 0x6; return;
            case SDLK_r: c->key = 0xD; return;
            case SDLK_a: c->key = 0x7; return;
            case SDLK_s: c->key = 0x8; return;
            case SDLK_d: c->key = 0x9; return;
            case SDLK_f: c->key = 0xE; return;
            case SDLK_z: c->key = 0xA; return;
            case SDLK_x: c->key = 0x0; return;
            case SDLK_c: c->key = 0xB; return;
            case SDLK_v: c->key = 0xF; return;
            default:
                continue;
        }
    }

    return;
}

void chip8_renderdisplay(chip8_t *c) {
    SDL_SetRenderDrawColor(c->sdl->renderer, 0, 0, 0, 255);
    SDL_RenderClear(c->sdl->renderer);

    SDL_SetRenderDrawColor(c->sdl->renderer, 255, 255, 255, 255);

    for(size_t row = 0; row < DISPLAY_HEIGHT; ++row) {
        for(size_t col = 0; col < DISPLAY_WIDTH; ++col) {
            if(!c->display[row][col]) { continue; }

            SDL_Rect rect = {
                col * SCALE,
                row * SCALE,
                SCALE,
                SCALE
            };
            SDL_RenderFillRect(c->sdl->renderer, &rect);
        }
    } 

    SDL_RenderPresent(c->sdl->renderer);
}

void chip8_exec(chip8_t *c) {
    uint16_t pc = c->pc;
    uint16_t instruction = chip8_fetch(c);
    
    #define DEBUG

    #ifdef DEBUG
        printf("PC: 0x%04x Op: 0x%04x\n", pc, instruction);
    #endif
    
    // CLS — 00E0
    if(instruction == 0x00E0) {
        chip8_cleardisplay(c);
        c->pc += 2;
        return;
    }

    // RET — 00EE
    if(instruction == 0x00EE) {
        c->pc = chip8_pop(c);
        return;
    }

    // JMP — 1NNN
    if(msn(msb(instruction)) == 0x1) {
        c->pc = instruction & 0x0FFF;         
        return;
    }

    // CALL NNN — 2NNN
    if(msn(msb(instruction)) == 0x2) {
        chip8_push(c, c->pc + 2);    
        c->pc = instruction & 0xFFF;         
        return;
    }

    // SE VX, NN — 3XNN
    if(msn(msb(instruction)) == 0x3) {
        size_t x = (instruction >> (4 * 2)) & 0xF;
        uint8_t nn = instruction & 0xFF;
        
        c->pc += (c->regs[x] == nn) ? 4 : 2;
        return;
    }

    // SNE VX, NN — 4XNN
    if(msn(msb(instruction)) == 0x4) {
        size_t x = (instruction >> (4 * 2)) & 0xF;
        uint8_t nn = instruction & 0xFF;
        
        c->pc += (c->regs[x] != nn) ? 4 : 2;
        return;
    }

    // SE VX, VY — 5XY0
    if(msn(msb(instruction)) == 0x5) {
        char xy = mb(instruction);
        size_t x = (size_t) msn(xy);
        size_t y = (size_t) lsn(xy);

        c->pc += (c->regs[x] == c->regs[y]) ? 4 : 2;
        return;
    }

    // LD VX, NN — 6XNN
    if(msn(msb(instruction)) == 0x6) {
        size_t x = (size_t) lsn(msb(instruction));
        uint8_t nn = (uint8_t) lsb(instruction);
        c->regs[x] = nn;
        c->pc += 2;
        return;
    }

    // ADD VX, NN — 7XNN
    if(msn(msb(instruction)) == 0x7) {
        size_t x = (size_t) lsn(msb(instruction));
        uint8_t nn = (uint8_t) lsb(instruction);
        c->regs[x] += nn;
        c->pc += 2;
        return;
    }

    // LD VX, VY — 8XY0
    if(msn(msb(instruction)) == 0x8) {
        uint8_t xy = (uint8_t) mb(instruction);
        size_t x = (size_t) msn(xy);
        size_t y = (size_t) lsb(xy);
    
        switch(lsn(lsb(instruction))) {
            // LD VX, VY — 8XY0
            case 0x0:
                c->regs[x] = c->regs[y];
                break;
            // OR VX, VY — 8XY1
            case 0x1:
                c->regs[x] |= c->regs[y];
                break;
            // AND VX, VY — 8XY2
            case 0x2:
                c->regs[x] &= c->regs[y];
                break;
            // XOR VX, VY — 8XY3
            case 0x3:
                c->regs[x] ^= c->regs[y];
                break;
            // ADD VX, VY — 8XY4
            case 0x4:
                c->regs[vf] = (uint8_t)(((int16_t)c->regs[x] + (int16_t)c->regs[y]) > 0xFF);
                c->regs[x] += c->regs[y];
                break;
            // SUB VX, VY — 8XY5
            case 0x5:
                c->regs[vf] = (uint8_t)(c->regs[x] > c->regs[y]);
                c->regs[x] -= c->regs[y];
                break; 
            // SHR VX {, VY} — 8XY6
            case 0x6:
                c->regs[vf] = c->regs[x] & 0x01;
                c->regs[x] >>= 1;
                break;
            // SUBN VX, VY — 8XY7
            case 0x7:
                c->regs[vf] = (uint8_t)(c->regs[y] > c->regs[x]);
                c->regs[x] = c->regs[y] - c->regs[x];
                break;
            // SHL VX {, VY} — 8XYE
            case 0xE:
                c->regs[vf] = (uint8_t)(c->regs[x] & 0x80);
                c->regs[x] <<= 1;
                break;
        }

        c->pc += 2;
        return;
    }

    // SNE VX, VY — 9XY0
    if(msn(msb(instruction)) == 0x9) {
        uint8_t xy = (uint8_t)mb(instruction);
        size_t x = (size_t) msn(xy);
        size_t y = (size_t) lsn(xy);
        
        
        c->pc += (c->regs[x] != c->regs[y]) ? 4 : 2;
        return;        
    }

    // LD I, NNN — ANNN
    if(msn(msb(instruction)) == 0xA) {
        c->i = (uint16_t)(((lsn(msb(instruction)) << (4 * 2)) + lsb(instruction)) & 0xFFF);
        c->pc += 2;
        return;
    }

    // JMP V0, NNN — BNNN
    if(msb(msb(instruction)) == 0xB) {
        c->pc = c->regs[0] + (uint16_t)(((lsn(msb(instruction)) << (4 * 2)) + lsb(instruction)) & 0xFFF);
        return;
    }

    // RND VX, NN – CXNN
    if(msn(msb(instruction)) == 0xC) {
        size_t x = (size_t)lsn(msb(instruction));
        uint8_t nn = (uint8_t)lsb(instruction);
        c->regs[x] = rand() & nn;
        c->pc += 2;
        return;
    }

    // DRW VX, VY, N — DXYN
    if(msn(msb(instruction)) == 0xD) {
        uint8_t xy = mb(instruction);
        size_t x = (size_t) msn(xy);
        size_t y = (size_t) lsn(xy);
        size_t n = (size_t) lsn(lsb(instruction));

        size_t xcoord = (size_t)(c->regs[x]) % DISPLAY_WIDTH;
        size_t ycoord = (size_t)(c->regs[y]) % DISPLAY_HEIGHT;

        for(size_t row = 0; row < n; ++row) {
            uint8_t byte = c->mem[c->i + row];
            size_t cy = (ycoord + row) % DISPLAY_HEIGHT;

            for(size_t col = 0; col < 8; ++col) {
                size_t cx = (xcoord + col) % DISPLAY_WIDTH;
                bool bit = byte & (0x80 >> col); 

                if (bit) {
                    if (c->display[cy][cx]) c->regs[vf] = 1;
                    c->display[cy][cx] ^= 1;
                }
                
                if(cx == DISPLAY_WIDTH -1) { break; }
            }

            if(cy == DISPLAY_HEIGHT - 1) { break; }
        }

        c->pc += 2;
        return;
    }


    if(msn(msb(instruction) == 0xE)) {
        size_t x = (size_t)msb(lsn(instruction));
        switch(lsb(instruction)) {
            // SKP VX — EX9E
            case 0x9E:
                if(c->keys[c->regs[x]]) { c->pc += 2; }
                break;
            // SKNP VX — EXA1
            case 0xA1:
                if(!c->keys[c->regs[x]]) { c->pc += 2; }
                break;
        }

        c->pc += 2;
        return;
    }

    if(msn(msb(instruction)) == 0xF) {
        size_t x = (size_t)lsn(msb(instruction));
        switch(lsb(instruction)) {
            // LD VX, DT — FX07
            case 0x07:
                c->regs[x] = c->dt;
                break;
            // LD VX, K — FX0A
            case 0x0A:
                chip8_readkey(c);
                c->regs[x] = c->key;
                break;
            // LD DT, VX — FX15
            case 0x15:
                c->dt = c->regs[x];
                break;
            // LD ST, VX — FX18
            case 0x18:
                c->st = c->regs[x];
                break;
            // ADD I, VX — FX1E
            case 0x1E:
                c->i += c->regs[x];
                break;
            // LD F, VX — FX29
            case 0x29:
                c->i = c->regs[x] * 0x05;
                break;
            // LD B, VX — FX33
            case 0x33:
                uint8_t h = c->regs[x] / 100;
                uint8_t t = c->regs[x] / 10;
                uint8_t o = c->regs[x] - h * 100 - t * 10;

                c->mem[c->i] = h;
                c->mem[c->i + 1] = t;
                c->mem[c->i + 2] = o;
                break;
            // LD [I], VX — FX55
            case 0x55:
                for(size_t reg = 0; reg <= x; ++reg) { c->mem[c->i + reg] = c->regs[reg]; }
                break;
            // LD VX, [I] — FX65
            case 0x65:
                for(size_t reg = 0; reg <= x; ++reg) { c->regs[reg] = c->mem[c->i + reg]; }
                break;
        }

        c->pc += 2;
        return;
    }
}