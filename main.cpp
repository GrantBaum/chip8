#include <SDL2/SDL.h>
#include <iostream>
#include <iomanip>
#include "chip8.h"

int main() {
    Chip8 chip;

    for (int i = 0; i < 80; i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << (int)chip.getByte(0x050 + i) << " ";
    if (i % 5 == 4) std::cout << "\n";
}
}