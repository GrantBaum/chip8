#pragma once
#include <array>
#include <stack>
#include <cstdint>

class Chip8{

    friend struct Chip8Test;

    public:
        Chip8(); //constructor
    
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

        std::array<std::array<bool, 64>, 128> display{}; //monocrhome display as a 2d array (large size)
        uint16_t pc = 0x200; //program counter current instruction
        uint16_t i = 0; //index register to point to locs in memory
        uint8_t delay_timer = 0; //incrementing towards 0
        uint8_t sound_timer = 0; //sound timer (when not zero sound plays)
        std::array<bool, 16> key_state = {false}; //the state of each key
        std::array<uint8_t, 4096> memory{}; // 4KiB of contiguous memory
        std::stack<uint16_t> stack;
        std::array<uint8_t, 16> v{}; //variable register arr where index 15 is carry register (VF)

        int8_t held_key = -1; //FX0A: key we're waiting to be released, -1 means none

        //constants
        const uint16_t PGM_MAX_SIZE = 3584; //MAX PROGRAM SIZE IN BYTES
        const uint16_t FONT_ADDR = 0x050;
        static const uint16_t OP_MASK = 0xF000;
        static const uint16_t X_MASK = 0x0F00;
        static const uint16_t Y_MASK = 0x00F0;
        static const uint16_t N_MASK = 0x000F;
        static const uint16_t NN_MASK = 0x00FF;
        static const uint16_t NNN_MASK = 0x0FFF;

        uint16_t random_num(); //random number header declaration

        //screen and flow (returns what was pushed or pop)
        void clearScreen(); //00E0 - clear all pixels
        void push(uint16_t val); //2NNN - set pc to NNN, add to stack, return NNN
        void pop(); //00EE - pop last address from stack, set PC to that address, return address

        //conditional skips
        void skp_equals(uint8_t x, uint8_t val); //3XNN - will skip one instruction if the value in VX is equal to NN 
        void skp_nequals(uint8_t x, uint8_t val); //4XNN - will skip if they are not equal.
        void skp_iftrue(uint8_t x, uint8_t y); //5XY0 - skips if the values in VX and VY are equal
        void skp_iffalse(uint8_t x, uint8_t y); //9XY0 - skips if they are not equal.

        //registers
        void set_reg(uint8_t x, uint8_t val); //6XNN - sets register VX to value NN
        void add_reg(uint8_t x, uint8_t val); //7XNN - adds NN to val at register VX

        //math and logic (8XY_)
        void set_to_reg(uint8_t x, uint8_t y); //8XY0 - sets register at VX to VY
        void logic_or(uint8_t x, uint8_t y); //8XY1 - sets register at VX to VX | VY
        void logic_and(uint8_t x, uint8_t y); //8XY2 - sets register at VX to VX & VY
        void logic_xor(uint8_t x, uint8_t y); //8XY3 - sets register at VX to VX ^ VY
        void add(uint8_t x, uint8_t y); //8XY4 - sets VX to VX + VY. If overflow set VF to 1
        void subtr(uint8_t x, uint8_t y); //8XY5 - sets VX to VX - VY. VF = 1 if VX >= VY
        void rshift(uint8_t x); //8XY6 - Shift right by 1. VF = the bit shifted out
        void lshift(uint8_t x); //8XY7 - Shift left by 1. VF = the bit shifted out
        void reverse_subtr(uint8_t x, uint8_t y); //8XYE - set VX to VY - VX. VF = 1 if VY >= VX

        //index and memory (these will return indicies where I ends up or what gets stored)
        uint16_t set_index(uint16_t val); //ANNN - set I to NNN
        uint16_t add_index(uint8_t x); //FX1E - add VX to I
        uint16_t font_char(uint8_t x); //FX29 - set I to font char for a hex number in VX
        void decimal_conv(uint8_t x); //FX33 - store VX as 3 decimal digits at I, I+1 and I+2

        //heap memory
        /*this occurs when the registers need to be dumped in memory or vise versa. make sure it goes to the next availible address
        to not overwrite things, or throw if it tries, or let it, or reserve part of my first 512 bytes for it*/
        void cache_regs(uint8_t x); //FX55 - put regs V0 thru VF in memory starting at I and clear registers
        void fetch_regs(uint8_t x); //FX65 - load V0 thru VF into registers from location I

        //jumps and random
        uint16_t jump(uint16_t val); //1NNN - set pc to NNN, return nnn
        uint16_t jump_o(uint16_t val) //BNNN - set pc to NNN + V0, return pc
        void random(uint8_t x, uint8_t val); //CXNN - set VX to RANDOM_NUM & NN

        //drawing
        void draw(uint8_t x, uint8_t y, uint8_t val); //DXYN - Draw an N-row sprite from memory at I, at position (VX, VY). Pixels XOR onto the screen. VF = 1 if any pixel got turned off

        //keys
        void skip_if_key(uint8_t x); //EX9E - Skip next instruction if the key in VX is pressed
        void skip_not_key(uint8_t x); //EX1A - Skip next instruction if the key in VX is not pressed
        void wait_key(uint8_t x); //FX0A - Wait for a key, then store it in VX

        //timers
        void get_dt(uint8_t x); //FX07 - set VX to delay timer
        void set_dt(uint8_t x); //FX15 - set delay timer to VX
        void set_st(uint8_t x); //FX18 - set sound timer to VX
        
};
