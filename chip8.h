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
       bool exec(uint16_t code); //returns true unless fail

    private:
        bool display[128][64] = {false}; //monocrhome display as a 2d array (large size)

        uint16_t pc = 0x200; //program counter current instruction
        uint16_t i = 0; //index register to point to locs in memory

        uint8_t delay_timer = 0; //incrementing towards 0
        uint8_t sound_timer = 0; //sound timer (when not zero sound plays)

        bool key_state[16] = {false}; //the state of each key
        std::array<uint8_t, 4096> memory{}; // 4KiB of contiguous memory
        std::stack<uint16_t> stack;
        std::array<uint8_t, 16> v{}; //variable register arr
        
};
