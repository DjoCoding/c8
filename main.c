#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <c8.h>
#include <sdl.h>


sdl_t *sdl = NULL;

chip8_program_t programof(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if(!f) {
        printf("failed to open file %s\n", filename);
        return (chip8_program_t) {
            NULL,
            0
        };
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *content = malloc(size);
    if(!content) {
        printf("malloc Error");
        return (chip8_program_t) {
            NULL,
            0
        };
    }

    size_t n = fread(content, 1, size, f);
    if(n != size) {
        printf("failed to read file %s\n", filename);
        free(content);
        return (chip8_program_t) {
            NULL,
            0
        };
    }


    return (chip8_program_t) {
        content,
        size
    };
}

int main(int argc, char **argv) {
    srand(time(NULL));

    sdl = sdl_init();
    if(!sdl) {
        return 1;
    }

    char *filepath = argv[1];
    if(!filepath) {
        printf("no chip8 program listed\n");
        sdl_quit(sdl);
        exit(EXIT_FAILURE);
    }

    chip8_program_t program = programof(filepath);
    if(program.content == 0) {
        sdl_quit(sdl);
        exit(EXIT_FAILURE);
    }

    chip8_t c = chip8_init(sdl, program);

    bool quit = false;
    SDL_Event event = {0};

    while(!quit) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                quit = true;
                break;
            }
        }

        chip8_exec(&c);
        chip8_renderdisplay(&c);
    }

    sdl_quit(sdl);
    return 0;
}