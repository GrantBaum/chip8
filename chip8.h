#pragma once
#include <array>
#include <stack>
#include <cstdint>

class Chip8{
    public:
        Chip8(); //constructor
        const uint16_t PGM_MAX_SIZE = 3584; //MAX PROGRAM SIZE IN BYTES
        const uint16_t FONT_ADDR = 0x050;

        //byte methods
        uint8_t getByte(uint16_t addr) const; //read only and only one that returns int
        bool setByte(uint16_t addr, uint8_t byte); //needs to know what to set
        bool clearByte(uint16_t addr);

        //key methods
        void setKey(uint8_t key);
        void clearKey(uint8_t key);

        //display methods
        bool getBit(int x, int y) const; //read only
        void setBit(int x, int y);
        void clearBit(int x, int y);

       //other helpers
       void clock();
       bool play_sound();
       void load(const char *program); //returns true unless fail
       bool exec(); //returns true unless fail

    private:
    
        bool display[128][64] = {false}; //monocrhome display as a 2d array (large size)
        uint16_t pc = 0x200; //program counter current instruction
        uint16_t i = 0; //index register to point to locs in memory
        uint8_t delay_timer = 0; //incrementing towards 0
        uint8_t sound_timer = 0; //sound timer (when not zero sound plays)
        bool key_state[16] = {false}; //the state of each key
        std::array<uint8_t, 4096> memory{}; // 4KiB of contiguous memory
        std::stack<uint16_t> stack;
        std::array<uint8_t, 16> v{}; //variable register arr where index 15 is carry register (VF)



        //screen and flow (returns status and what was pushed or pop)
        bool clearScreen(); //00E0 - clear all pixels
        uint16_t push(); //2NNN - set pc to NNN, add to stack, return NNN
        uint16_t pop(); //00EE - pop last address from stack, set PC to that address, return address

        //conditional skips (returns evaluation of the prompt)
        bool equals(uint8_t val); //3XNN - will skip one instruction if the value in VX is equal to NN 
        bool not_equals(uint8_t val); //4XNN - will skip if they are not equal.
        bool compare_true(); //5XY0 - skips if the values in VX and VY are equal
        bool compare_false(); //9XY0 - skips if they are not equal.

        //registers (returns true if sucessful else false)
        bool set_reg(uint8_t val); //6XNN - sets register VX to value NN
        bool add_reg(uint8_t val); //7XNN - adds NN to val at register VX

        //math and logic (8XY_) they return true if successful otherwise false
        bool set_to_reg(); //8XY0 - sets register at VX to VY
        bool logic_or(); //8XY1 - sets register at VX to VX | VY
        bool logic_and(); //8XY2 - sets register at VX to VX & VY
        bool logic_xor(); //8XY3 - sets register at VX to VX ^ VY
        bool add(); //8XY4 - sets VX to VX + VY. If overflow set VF to 1
        bool subtr(); //8XY5 - sets VX to VX - VY. VF = 1 if VX >= VY
        bool rshift(); //8XY6 - Shift right by 1. VF = the bit shifted out
        bool lshift(); //8XY7 - Shift left by 1. VF = the bit shifted out
        bool reverse_subtr(); //8XYE - set VX to VY - VX. VF = 1 if VY >= VX

        //index and memory (these will return indicies where I ends up or what gets stored)
        uint16_t set_index(); //ANNN - set I to NNN
        uint16_t add_index(); //FX1E - add VX to I
        uint16_t font_char(); //FX29 - set I to font char for a hex number
        uint8_t decimal_conv(); //FX33 - store VX as 3 decimal digits at I, I+1 and I+2

        //heap memory
        /*this occurs when the registers need to be dumped in memory or vise versa. make sure it goes to the next availible address
        to not overwrite things, or throw if it tries, or let it, or reserve part of my first 512 bytes for it*/
        bool cache_regs(); //FX55 - put regs V0 thru VF in memory starting at I and clear registers
        bool fetch_regs(); //FX65 - load V0 thru VF into registers from location I

        //jumps and random
        uint16_t jump(); //1NNN - set pc to NNN, return nnn
        uint16_t random(); //CXNN - set VX to RANDOM_NUM & NN

        //drawing
        void draw(); //DXYN - Draw an N-row sprite from memory at I, at position (VX, VY). Pixels XOR onto the screen. VF = 1 if any pixel got turned off

        //keys
        bool skip_if_key(); //EX9E - 
        bool skip_not_key(); //EX1A - 
        bool wait_key(); //FX0A - 

        //timers
        uint8_t get_dt(); //FX07 - set VX to delay timer
        uint8_t set_dt(); //FX15 - set delay timer to VX
        uint8_t set_st(); //FX18 - set sound timer to VX
        
};
