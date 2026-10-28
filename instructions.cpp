#include "chip8.h"
#include <random>

//random number gen
uint16_t Chip8::random_num() {
    //static makes sure these are only initialized once
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_int_distribution<uint16_t> distrib(0, 255);
    return distrib(gen);
}

//screen and flow
void Chip8::clearScreen(){
    display = {}; //sets everything to false or 0
}
void Chip8::push(uint16_t val){
    stack.push(pc);
    pc = val; //nnn
}
void Chip8::pop(){
    pc = stack.top();
    stack.pop();
}

//conditional skips
void Chip8::skp_equals(uint8_t x, uint8_t val){
    if(v.at(x) == val){ //nn
        pc += 2;
    }
}
void Chip8::skp_nequals(uint8_t x, uint8_t val){
    if(v.at(x) != val){ //nn
        pc += 2;
    }
}
void Chip8::skp_iftrue(uint8_t x, uint8_t y){
    if(v.at(x) == v.at(y)){
        pc += 2;
    }
}
void Chip8::skp_iffalse(uint8_t x, uint8_t y){
    if(v.at(x) != v.at(y)){
        pc += 2;
    }
}

//registers
void Chip8::set_reg(uint8_t x, uint8_t val){
    v.at(x) = val; //nn
}
void Chip8::add_reg(uint8_t x, uint8_t val){
    v.at(x) += val; //nn
}

//math and logic
void Chip8::set_to_reg(uint8_t x, uint8_t y){
    v.at(x) = v.at(y);
}
void Chip8::logic_or(uint8_t x, uint8_t y){
    v.at(x) = v.at(x) | v.at(y);
}
void Chip8::logic_and(uint8_t x, uint8_t y){
    v.at(x) = v.at(x) & v.at(y);
}
void Chip8::logic_xor(uint8_t x, uint8_t y){
    v.at(x) = v.at(x) ^ v.at(y);
}
void Chip8::add(uint8_t x, uint8_t y){
    uint16_t sum = v.at(x) + v.at(y);
    if(sum > 255){
        v.at(0xF) = 1; //set the carry bit
    }
    else v.at(0xF) = 0;

    if(x != 0xF){
        v.at(x) = static_cast<uint8_t>(sum);
    }
}
void Chip8::subtr(uint8_t x, uint8_t y){
    if(v.at(y) <= v.at(x)){
        v.at(0xF) = 1;
    }
    else v.at(0xF) = 0;

    if(x != 0xF) v.at(x) = v.at(x) - v.at(y);
    
}
void Chip8::rshift(uint8_t x){
    ((v.at(x) & 0x01) != 0) ? v.at(0xF) = 1 : v.at(0xF) = 0;

    if(x != 0xF) v.at(x) = v.at(x) >> 1;
}
void Chip8::lshift(uint8_t x){
    ((v.at(x) & 0x80) != 0) ? v.at(0xF) = 1 : v.at(0xF) = 0;

    if(x != 0xF) v.at(x) = v.at(x) << 1;
}
void Chip8::reverse_subtr(uint8_t x, uint8_t y){
    if(v.at(x) <= v.at(y)){
        v.at(0xF) = 1;
    }
    else v.at(0xF) = 0;

    if(x != 0xF) v.at(x) = v.at(y) - v.at(x);
}

//index and memory
uint16_t Chip8::set_index(uint16_t val){
    i = val; //nnn
    return i;
}
uint16_t Chip8::add_index(uint8_t x, uint8_t val){
    i += v.at(x);
    return i;
}
uint16_t Chip8::font_char(uint8_t x){
    i = FONT_ADDR + (v.at(x) & 0x0F) * 5;
    return i;
}
}
void Chip8::decimal_conv(uint8_t x){
    uint8_t val = v.at(x);
    memory.at(i)     = val / 100;         //hundreds
    memory.at(i + 1) = (val / 10) % 10;   //tens
    memory.at(i + 2) = val % 10;          //ones
}

//heap memory
void Chip8::cache_regs(uint8_t x){
    for(int j = 0; j <= x; j++, i++){
        memory.at(i) = v.at(j);
    }
}
void Chip8::fetch_regs(uint8_t x){
    for(int j = 0; j <= x; j++, i++){
        v.at(j) = memory.at(i);
    }
}

//jumps and random
uint16_t Chip8::jump(uint16_t val){
    pc = val;
    return pc;
}
uint16_t Chip8::random(uint8_t x){
    return random_num() & v.at(x);
}

//drawing
void Chip8::draw(){
    //TODO
}

//keys
void Chip8::skip_if_key(uint8_t x, uint8_t key){
    if(key_state.at(v.at(x)) == true){
        pc += 2;
    }
}
void Chip8::skip_not_key(uint8_t x, uint8_t key){
    if(key_state.at(v.at(x)) == false){
        pc += 2;
    }
}
void Chip8::wait_key(uint8_t x, uint8_t key){
    while(true){
        if(key_state.at(v.at(x)) == false){
            continue;
        }
        else break;
    }
}

//timers
void Chip8::get_dt(){
    v.at(x) = delay_timer;
}
void Chip8::set_dt(uint8_t x){
    delay_timer = v.at(x);
}
void Chip8::set_st(uint8_t x){
    sound_timer = v.at(x);
}