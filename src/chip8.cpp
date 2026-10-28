#include "chip8.h"
#include "font.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdexcept> // Required for standard exceptions
#include <format>

//implementation of public methods from the class
Chip8::Chip8(){
    i = FONT_ADDR;
    for(size_t j = 0; j < font.size(); j++, i++){
        memory.at(i) = font.at(j);
    }

    i = 0; //resetting register
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

void Chip8::clock(){
    if(delay_timer > 0) delay_timer--;
    if(sound_timer > 0) sound_timer--;
}

bool Chip8::play_sound(){
    if(sound_timer != 0){
        return true;
    }
    else return false;
}
void Chip8::load(const char *program){
    std::ifstream rom(program, std::ios::binary);

    if(!rom.is_open()) {
        throw std::runtime_error("Failed to open ROM: " + std::string(program));
    }

    rom.seekg(0, std::ios::end); //jump to the end to get the size
    size_t fileSize = static_cast<size_t>(rom.tellg());
    rom.seekg(0, std::ios::beg); //back to the start for reading

    if(fileSize > PGM_MAX_SIZE){
        throw std::runtime_error("ROM too big: " + std::string(program));
    }

    char byte;
    uint16_t addr = 0x200; //programs start here
    while(rom.get(byte)){
        memory.at(addr) = static_cast<uint8_t>(byte);
        addr++;
    }

    std::cout << "Loaded " << std::dec << fileSize << " bytes. Next free address: 0x" << std::hex << addr << std::dec << std::endl;
}

void Chip8::exec(){
    //process of setting the code to the instruction
    uint16_t code = memory.at(pc);
    pc++;
    code = code << 8;
    code = code | memory.at(pc);
    pc++;

    uint16_t op_nibble = (code & OP_MASK) >> 12;
    uint16_t x = (code & X_MASK) >> 8;
    uint16_t y = (code & Y_MASK) >> 4;
    uint16_t n = code & N_MASK;
    uint16_t nn = code & NN_MASK;
    uint16_t nnn = code & NNN_MASK;

    switch(op_nibble){
        case 0x0:
            if(nn == 0xEE){
                pop(); //00EE
            }
            else if(nn == 0xE0){
                clearScreen(); //00E0
            }
            else throw std::runtime_error(std::format("Invalid instruction: {:04X} at pc {:03X}", code, pc - 2));
            break;

        case 0x1: //1NNN
            jump(nnn);
            break;

        case 0x2: //2NNN
            push(nnn);
            break;

        case 0x3: //3XNN
            skp_equals(x, nn);
            break;

        case 0x4: //4XNN
            skp_nequals(x, nn);
            break;

        case 0x5: //5XY0
            skp_iftrue(x, y);
            break;

        case 0x6: //6XNN
            set_reg(x, nn);
            break;

        case 0x7: //7XNN
            add_reg(x, nn);
            break;

        case 0x8:
            if(n == 0){
                set_to_reg(x, y); //8XY0
            }
            else if(n == 0x1){
                logic_or(x, y); //8XY1
            }
            else if(n == 0x2){
                logic_and(x, y); //8XY2
            }
            else if(n == 0x3){
                logic_xor(x, y); //8XY3
            }
            else if(n == 0x4){
                add(x, y); //8XY4
            }
            else if(n == 0x5){
                subtr(x, y); //8XY5
            }
            else if(n == 0x6){
                rshift(x); //8XY6
            }
            else if(n == 0x7){
                lshift(x); //8XY7
            }
            else if(n == 0xE){
                reverse_subtr(x, y); //8XYE
            }
            else throw std::runtime_error(std::format("Invalid instruction: {:04X} at pc {:03X}", code, pc - 2));
            break;

        case 0x9: //9XY0
            skp_iffalse(x, y);
            break;

        case 0xA: //ANNN
            set_index(nnn);
            break;

        case 0xB: //BNNN
            jump_o(nnn);
            break;

        case 0xC: //CXNN
            random(x, nn);
            break;

        case 0xD: //DXYN
            draw(x, y, n);
            break;

        case 0xE:
            if(nn == 0x9E){
                skip_if_key(x); //EX98
            }
            else if(nn == 0xA1){
                skip_not_key(x); //EX1A
            }
            else throw std::runtime_error(std::format("Invalid instruction: {:04X} at pc {:03X}", code, pc - 2));
            break;

        case 0xF:
            if(nn == 0x07){
                get_dt(x); //FX07
            }
            else if(nn == 0x15){
                set_dt(x); //FX15
            }
            else if(nn == 0x18){
                set_st(x); //FX18
            }
            else if(nn == 0x29){
                font_char(x); //FX29
            }
            else if(nn == 0x33){
                decimal_conv(x); //FX33
            }
            else if(nn == 0x55){
                cache_regs(x); //FX55
            }
            else if(nn == 0x65){
                fetch_regs(x); //FX65
            }
            else if(nn == 0x0A){
                wait_key(x); //FX0A
            }
            else if(nn == 0x1E){
                add_index(x); //FX1E
            }
            else throw std::runtime_error(std::format("Invalid instruction: {:04X} at pc {:03X}", code, pc - 2));
            break;

        default:
            throw std::runtime_error("Unknown instruction error occured!");
            break;
    }
}
