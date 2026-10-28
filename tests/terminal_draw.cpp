#include "chip8.h"
#include <iostream>
#include <string>

// Temporary main: runs a ROM for a set number of cycles, then prints the screen.
// # is on, . is off.
//
//   ./chip8                              (IBM logo, 20 cycles)
//   ./chip8 roms/1-chip8-logo.ch8 39     (CHIP-8 logo, 39 cycles)

int main(int argc, char* argv[]) {
    const char* path = "roms/2-ibm-logo.ch8";
    int cycles = 20;

    if (argc >= 2) path = argv[1];
    if (argc >= 3) cycles = std::stoi(argv[2]);

    Chip8 chip;
    chip.load(path);
    
    for (int c = 0; c < cycles; c++) {
        chip.exec();
    }

    std::cout << path << " after " << cycles << " cycles:\n\n";
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            std::cout << (chip.getBit(x, y) ? '#' : '.');
        }
        std::cout << "\n";
    }

    return 0;
}