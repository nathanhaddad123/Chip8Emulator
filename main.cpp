
#include <iostream>
#include "Chip8.hpp"

int main( ) {
    
    //Initialize variables
    //(Only works cuz i set the working directory to root then put c8games folder in root)
    char* path = "Documents/c8games/PONG";
    
    timespec time1, time2;
    time1.tv_sec = 0;
    time1.tv_nsec= 5000000;
    
    //Initialize Chip-8
    Chip8 myChip8;
    

    
    //Loading/initializing chip8 (load func calls init)
    myChip8.load(path);
    
    //Emulation loop
    while (true){
        
        myChip8.emulateCycle();

        myChip8.displayGraphics();
        
        nanosleep(&time1, &time2);
    }
    

    

}
