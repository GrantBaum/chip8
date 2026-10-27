#include "chip8.h"
#include "font.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdexcept> // Required for standard exceptions

//implementation of public methods from the class
Chip8::Chip8(){
    i = 0x050; //memory address is set to 50
    for(int j = 0; j < font.size(); j++, i++){
        memory.at(i) = font.at(j);
    }

    i = 0; //resetting register.
}

//byte methods
uint8_t Chip8::getByte(uint16_t addr) const{
    return memory.at(addr); //4096 addresses fits in 12 bytes. THERE NEEDS TO BE ERROR HANDLING HERE but do it later
}
bool Chip8::setByte(uint16_t addr, uint8_t byte){
    if(addr <= 4096){
        memory.at(addr) = byte;
        return true;
    }
    else return false; //address did not fit, ALWAYS THROW IN THIS CASE
}
bool Chip8::clearByte(uint16_t addr){
    if(addr <= 4096){
        memory.at(addr) = 0; //0 means free
        return true;
    }
    else return false; //ALWAYS THROW IN THIS CASE
}

void Chip8::setKey(uint8_t key){
    if(key < 0x10 && key > 0){
        key_state[key] = true;
    }
}
void Chip8::clearKey(uint8_t key){
    if(key < 0x10 && key > 0){
        key_state[key] = false;
    }
}

bool Chip8::getBit(int x, int y) const{
    if(x <= 128 && y <= 64){
        return display[x][y];
    }
    else throw std::runtime_error("Bit to retreive is out of range");
}
void Chip8::setBit(int x, int y){
    if(x <= 128 && y <= 64){
        display[x][y] = true;
    }
    else throw std::runtime_error("Bit to set is out of range");
}
void Chip8::clearBit(int x, int y){
    if(x <= 128 && y <= 64){
        display[x][y] = false;
    }
    else throw std::runtime_error("Bit to clear is out of range");
}

//void Chip8::clock();

bool Chip8::play_sound(){
    if(sound_timer != 0){
        return true;
    }
    else return false;
}
void Chip8::load(const char *program){
    std::ifstream rom; //starting ifstream called rom
    rom.open(program, std::ios::binary);

    if(!rom.is_open()) {
        std::cerr << "Failed to open program!" << std::endl;
    }

    size_t fileSize = static_cast<size_t>(rom.tellg()); //file size of program
    rom.seekg(0, std::ios::beg); //set reading pointer to beginning

    if(fileSize > PGM_MAX_SIZE){
        std::cerr << "Program does not fit into ROM!" << std::endl;
    }

    char byte;
    i = pc; //set the index register to where programs start
    while(rom.get(byte)){
        memory.at(i) = static_cast<uint8_t>(byte);
        i++;
    }

    std::cout << "Successfully loaded program! Next free index at address: " << std::hex << i << std::endl;
    i = 0; //reset index register.
    
}

//bool Chip8::exec(uint8_t code);