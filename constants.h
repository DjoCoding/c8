#define KB                      1024

#define CHIP8_MEMORY_SPACE      4 * KB
#define CHIP8_PROGRAMS_OFFSET   0x200
#define CHIP8_REGISTERS_COUNT   16
#define CHIP8_STACK_SIZE        100

#define DISPLAY_WIDTH   64
#define DISPLAY_HEIGHT  32
#define SCALE           20

#define FONT_SIZE       5
#define FONTS_COUNT     16
#define FONT_ARRAY_LEN  FONT_SIZE * FONTS_COUNT


// most significant nibble
#define msn(v) ((v >> 4) & 0xF)

// least significant nibble
#define lsn(v) (v & 0xF)

// most significan byte
#define msb(v) ((v >> (4 * 2)) & 0xFF)

// least significant byte
#define lsb(v) (v & 0xFF)

// middle byte
#define mb(v) ((v >> 4) & 0xFF)

#define vf 0xF
