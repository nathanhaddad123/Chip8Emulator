#include <cstdio>
#include <iostream>
#include <random>
#include "Chip8.hpp"

//This function initializes Chip8 memory and other components
//to 0. This is to get it ready to load in a ROM
void Chip8::initChip8(){

    //Sets program counter to 0x200, as that's where working memory start
    pc = 0x200;

    //Setting some memory to 0
    I = 0;
    delay_timer = 0;
    sound_timer = 0;
    sp = 0;

    //Clearing stack
    for (int i = 0; i < 16; i++){stack[i]=0;}

    //Clearing registers
    for (int i = 0; i < 16; i++){V[i]=0;}

    //Clearing display
    for (int i = 0; i < 64*32; i++){display[i]=0;}

    //Clearing memory
    for (int i = 0; i < 4096; i++){memory[i]=0;}

    //Loading fontset in first 80 slots of memory
    for (int i = 0; i < 80; i++){memory[i] = chip8_fontset[i];}


}

// Initialise and load ROM into memory
bool Chip8::load(const char *path) {
    // Initialise
    initChip8();

    printf("Loading ROM: %s\n", path);
    
    errno = 0;
    // Attempt to open ROM file
    FILE* rom = fopen(path, "r");
    if (rom == NULL) {
        std::cerr << "Failed to open ROM" << std::endl;
        printf("Error %d \n", errno);
        return false;
    }
    

    // Get file size
    fseek(rom, 0, SEEK_END);
    long rom_size = ftell(rom);
    rewind(rom);

    // Allocate memory to store rom
    char* rom_buffer = (char*) malloc(sizeof(char) * rom_size);
    if (rom_buffer == NULL) {
        std::cerr << "Failed to allocate memory for ROM" << std::endl;
        return false;
    }

    // Copy ROM into buffer
    size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
    if (result != rom_size) {
        std::cerr << "Failed to read ROM" << std::endl;
        return false;
    }

    // Copy buffer to memory (ignoring the initial 512 bytes needed for reserved memory)
    if (rom_size < 3584){
        for (int i = 0; i < rom_size; ++i) {
            //Load memory into memory from 0x200 (i.e. 512)
            memory[i + 512] = (uint8_t)rom_buffer[i];
        }
    }
    else {
        std::cerr << "ROM too large to fit in memory" << std::endl;
        return false;
    }

    // Clean up
    fclose(rom);
    free(rom_buffer);
    
    std::cout << "ROM loaded!" << std::endl;

    return true;
}


/*This function emulates a cycle of Chip8. A cycle has 3 steps:
 1. Fetch Opcode: We will fetch the 2 byte opcodes one byte at a
 time and then merge them.

 2. Decode Opcode: Read the opcode a nibble at a time, as each
 nibble gives us identifying information.

 3. Execute Opcode: In this phase, memory is manipulated in order
 to emulate the result of an opcode.
 */


void Chip8::emulateCycle(){

    //FETCH OPCODE: Opcode is two bytes (4 nibs), and since we must
    //fetch each byte one at a time, we have to merge em to get
    //the whole opcode.
    //Ex:   memory[pc] = 01100101
    //      memory[pc] = 10001101
    //      memory[pc] << 8 = 0110010100000000
    //      memory[pc] << 8 | memory[pc+1] = 0110010110001101

    //Resetting drawFlag
    drawFlag = false;
    
    currentOp = memory[pc] << 8 | memory[pc+1];

    //DECODING AND EXECUTING OPCODE
    //Checking nibs at a time to avoid unneccesary comparisons

    //Check first nib of opcode...
    switch (currentOp & 0xF000){

        //Opcodes beginning with 0
        case 0x0000:
        {

            //Check last two nibs of opcodes beginning with 0
            switch (currentOp & 0x00FF){

                //00EE_ Return from subroutine
                case 0x00EE:
                {
                    pc = stack[sp];
                    sp--;
                    break;
                }

                //00E0_ Clear screen; set display elements to false
                case 0x00E0:
                {
                    for (int i = 0; i < 64*32; i++){
                        display[i] = false;
                    }
                    drawFlag = true;
                    break;
                }

                default:
                {
                    printf("Invalid opcode.");
                    break;
                }
            }
            break;
        }
            //End first nib = 0

        //1nnn: Jump to address nnn
        case 0x1000:
        {
            pc = currentOp & 0x0FFF;
            pc -= 2; //Do this because it's gonna increment it by two
                     //once we are done with the switch case
            break;
        }

        //2nnn: Call subroutine at nnn, must save current program
        case 0x2000:
        {
            sp++;
            stack[sp] = pc;
            pc = currentOp & 0x0FFF;
            break;
        }

        //3Xkk: Skip next instruction if Vx == kk
        case 0x3000:
        {
            uint8_t kk = currentOp & 0x00FF;
            uint8_t reg = (currentOp & 0x0F00) >> 8;
            if (V[reg] == kk)
                pc = pc + 2;//Each instruction is two bytes
            break;
        }

        //4Xkk: Skip next instruction if Vx != kk
        case 0x4000:
        {
            uint8_t kk = currentOp & 0x00FF;
            uint8_t reg = (currentOp & 0x0F00) >> 8;
            if (V[reg] != kk)
                pc = pc + 2;//Each instruction is two bytes
            break;
        }

        //5xy0: Skip nect intruction if Vx == Vy
        case 0x5000:
        {
            uint8_t regX = (currentOp & 0x0F00) >> 8;
            uint8_t regY = (currentOp & 0x00F0) >> 4;
            if (V[regX] == V[regY])
                pc += 2;//Each instruction is two bytes
            break;
        }

        //6xkk: Loads kk into Vx (LD Vx, kk)
        case 0x6000:
        {
            uint8_t kk = currentOp & 0x00FF;
            uint8_t reg = (currentOp & 0x0F00) >> 8;
            V[reg] = kk;
            break;
        }

        //7xkk: Adds value kk into register Vx (ADD Vx, kk)
        case 0x7000:
        {
            uint8_t kk = currentOp & 0x00FF;
            uint8_t reg = (currentOp & 0x0F00) >> 8;
            V[reg] += kk;
            break;
        }

        //8xy_: These are enumerated operations (math/logic) between registers
        case 0x8000:
        {
            //Getting registers
            uint8_t regX = (currentOp & 0x0F00) >> 8;
            uint8_t regY = (currentOp & 0x00F0) >> 4;

            //Check which operation to perform between them
            switch (currentOp & 0x000F)
            {
                    
                //8xy0: LD VX,VY
                case 0x0000:
                {
                    V[regX] = V[regY];
                    break;
                }

                //8xy1: Vx = Vx OR Vy
                case 0x0001:
                {
                    V[regX] |= V[regY];
                    break;
                }

                //8xy2: Vx = Vx AND Vy
                case 0x0002:
                {
                    V[regX] &= V[regY];
                    break;
                }

                //8xy3: Vx = Vx XOR Vy
                case 0x0003:
                {
                    V[regX] ^= V[regY];
                    break;
                }

                //8xy4: Vx = Vx ADD Vy, sets Vf to carry if needed
                case 0x0004:
                {
                    uint16_t result = V[regX] + V[regY];
                    V[regX] = result;
                    if (result > 255)
                        V[0xF] = true;
                    break;
                }

                //8xy5: Vx = Vx - Vy, sets Vf to NOT borrow if needed
                case 0x0005:
                {
                    if (V[regX] >= V[regY])
                        V[0xF] = true;

                    uint16_t result = V[regX] - V[regY];
                    V[regX] = result;
                    break;
                }

                //8xy6: Shifts Vx right then stores what got shifted out in Vf
                case 0x0006:
                {
                    if (V[regX] & 0x1) //if LSB is 1
                        V[0xF] = 1;
                    //This is essentially the same as right shifting as it also rounds down
                    V[regX] /= 2;
                    break;
                }

                //8xy7: Vx = Vy - Vx, sets Vf to NOT borrow if needed
                case 0x0007:
                {
                    if (V[regY] >= V[regX])
                        V[0xF] = true;

                    uint16_t result = V[regY] - V[regX];
                    V[regX] = result;
                    break;
                }

                //8xyE: Shifts Vx left then stores what got shifted out in Vf
                case 0x000E:
                {
                    if (V[regX] & 0x80) //if MSB is 1 (each reg is a Byte)
                        V[0xF] = 1;

                    //Doubling binary is arithmetic left shift
                    V[regX] *= 2;
                    break;
                }

                default:
                {
                    printf("Invalid opcode. (8)");
                    break;
                }
            }
            break;
        }

        //9xy0: (SNE Vx, Vy) Skip next instruction if Vx != Vy
        case 0x9000:
        {
            uint8_t regX = (currentOp & 0x0F00) >> 8;
            uint8_t regY = (currentOp & 0x00F0) >> 4;
            if (V[regX] != V[regY])
                pc += 2;//Each instruction is two bytes
            break;
        }

        //Annn: (LD I, nnn) Set I register to nnn
        case 0xA000:
        {
            I = currentOp & 0x0FFF;
            break;
        }

        //Bnnn: (JP V0, nnn) Jump to location nnn + v[0]
        case 0xB000:
        {
            pc = (currentOp & 0x0FFF) + V[0];
            pc -= 2; //Do this because it's gonna increment it by two
                     //once we are done with the switch case
            break;
        }

        //Cxkk: stores randNum & kk in V[x]
        case 0xC000:
        {
            uint8_t regX = currentOp & 0x0F00;
            std::random_device engine;  //random number generator
            uint8_t randNum = engine();
            uint8_t kk = currentOp & 0x00FF;

            V[regX] = randNum & kk;

            break;
        }

        //Dxyn: Draws an n-by-8 pixel sprite at (Vx, Vy)
        case 0xD000:
        {
            //Getting needed values
            uint8_t X = V[(currentOp & 0x0F00) >> 8];
            uint8_t Y = V[(currentOp & 0x00F0) >> 4];
            uint8_t spriteHeight = currentOp & 0x000F;
            uint8_t row;

            //Resetting collision flag
            V[0xF] = 0;
            
            //This loop reads one byte at a time from memory, starting at I
            for (int byte = 0; byte < spriteHeight; byte++){
                //This byte is a row of 8 pixels
                row = memory[I+byte];

                //This loop checks rows from left to right for pixels and draws them
                for (int pixel = 0; pixel < 8; pixel++){
                    
                    //If the pixel-th bit from the left is on
                    if ((row & (0x80 >> pixel)) != 0){
                        
                        //If that pixel is already occupied...
                        if (display[X + pixel + ((Y+byte)*64)] == 1){V[0xF] = 1;} //set collision to true}

                        //Draw the pixel
                        display[X + pixel + ((Y+byte)*64)] ^= 1;
                    }
                }
            }
            
            drawFlag = true;
            break;
        }
        
            //Exxx: These are opcode to check controls.
        case 0xE000:
        {
            uint8_t regX = (currentOp & 0x0F00) >> 8;
            switch (currentOp & 0x00FF)
            {
                //Ex9E: Skips to next instruction if key of value Vx is pressed
                case 0x009E:
                {
                    if ( keypad[V[regX]] )
                        pc +=2;
                    break;
                }

                //ExA1: Skips to next instruction if key of value Vx is NOT pressed
                case 0x00A1:
                {
                    if ( !keypad[ V[regX] ] )
                        pc +=2;
                    break;
                }
                    
                default:
                {
                    printf("Invalid opcode.");
                    break;
                }
            }
            break;
        }

        case 0xF000:
        {
            uint8_t regX = (currentOp & 0x0F00) >> 8;
            
            //Switching last two nibs  of Fxxx codes
            switch (currentOp & 0x00FF){

                //Fx07: Set Vx = delay timer
                case 0x07:
                {
                    V[regX] = delay_timer;
                    break;
                }

                //Fx0A: Wait til key is pressed, and store it in Vx
                case 0x0A:
                {
                    bool keyFlag = false;
                    for(int i = 0; i < 16; ++i)
                    {
                        if(keypad[i] != 0)
                        {
                            keyFlag = true;
                            V[regX] = i;
                        }
                    }

                    // If no key is pressed, return, keeping the pc at it's current value.
                    if(!keyFlag){
                        return;
                    }
                }

                //Fx15: Set delay timer to Vx
                case 0x15:
                {
                    delay_timer = V[regX];
                    break;
                }

                //Fx18: Set sound timer to Vx
                case 0x18:
                {
                    sound_timer = V[regX];
                    break;
                }

                //Fx1E: Set I = I + Vx
                case 0x1E:
                {
                    I += V[regX];
                    break;
                }

                //Fx29: Set I to location of sprite for digit at Vx
                case 0x29:
                {
                    I = V[regX] * 0x5;//0 is stored at memory address 0x0000, 1 is stored at 0x0005,...
                    break;
                }

                //Fx33: Stores Binary-coded decimal value at I, I+1, and I+2
                case 0x33:
                {
                    memory[I] = V[regX] / 100;          //hundreds digit
                    memory[I+1] = (V[regX] / 10) % 10;  //tens digit
                    memory[I+2] = V[regX] % 10;         //ones digit
                    break;
                }

                //Fx55: Stores V0 to Vx in memory starting at I
                case 0x55:
                {
                    //Putting V values into memory
                    for (int i = 0; i < regX; i++){
                        memory[I + i] = V[i];
                    }

                    //Incrementing I
                    I += I + regX + 1;
                    break;
                }

                //Fx65: Writes V0 to Vx from memory starting at I
                case 0x65:
                {
                    //Putting V values into memory
                    for (int i = 0; i < regX; i++){
                        V[i] = memory[I + i];
                    }

                    //Incrementing I
                    I += I + regX + 1;
                    break;
                }

                default:
                {
                    printf("Invalid opcode.");
                    break;
                }
            }
            
            break;
        }

        default:
        {
            printf("Invalid opcode.");
            break;
        }
    }

    //Incrementing program counter to read next instruction next cycle
    pc += 2;

    //Update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
        --sound_timer;

}

void Chip8::displayGraphics(){
    if (!drawFlag){return;}
    
    //This prints rows
    for (int i = 0; i < 32; i++){
        //this prints columns
        for (int j = 0; j < 64; j++){
            
            if (display[i*64 + j]){
                std::cout << "X";
            }
            else{
                std::cout << "`";
            }
        }
        std::cout << std::endl;
    }
    
}
