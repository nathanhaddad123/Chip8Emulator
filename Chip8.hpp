/*
 Author: Nathan Haddad
 
 This class emulates the Chip-8 system.

*/

#ifndef Chip8_hpp
#define Chip8_hpp

#include <cstdint>

class Chip8{
    
    //This stores the current opcode being executed
    uint16_t currentOp;
    
    //This is Chip8's 4KB memory
    uint8_t memory[4096];
    
    //These are the 16 8bit registers (Vx)
    uint8_t V[16];
    //NOTE: VF is not to be used by any program,
    //its used as a flag by certain ops such as draw sprite
    
    //This 16bit register stores memory addresses
    uint16_t I;
    
    //These special registers decrements at 60Hz,
    //used for delay and sound timers
    uint8_t delay_timer;
    uint8_t sound_timer;
    
    //Program counter; stores currently executing address
    uint16_t pc;
    
    //Stack; used to store pc before jump to other address,
    //sp stores the index of the top of the stack
    uint16_t stack[16];
    uint8_t sp;
    
    //Chip8 black & white display and draw flag
    bool display[64 * 32];
    bool drawFlag;
    
    //Chip8 HEX keypad
    bool keypad[16];
    
    //This function clears memory and screen, prepares variables
    void initChip8();
    
public:
    //This function loads and prepares roms for play.
    bool load(const char *path);
    
    //This function emulates one cycle of the emulator
    void emulateCycle();
    
    //This function displays the graphics in the command window
    void displayGraphics();

//This is the fontset for Chip8. Each row represents
//the alphanumeric comment to it's right.
uint8_t chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
    
};

#endif /* Chip8_hpp */

/*
//
 Memory Map from Cowgod's CHIP8 technical manual
 
 
 +---------------+= 0xFFF (4095) End of Chip-8 RAM
 |               |
 |               |
 |               |
 |               |
 |               |
 | 0x200 to 0xFFF|
 |     Chip-8    |
 | Program / Data|
 |     Space     |
 |               |
 |               |
 |               |
 +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
 |               |
 |               |
 |               |
 +---------------+= 0x200 (512) Start of most Chip-8 programs
 | 0x000 to 0x1FF|
 | Reserved for  |
 |  interpreter  |
 +---------------+= 0x000 (0) Start of Chip-8 RAM
 */
