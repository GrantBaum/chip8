// tests/test_instructions.cpp
// Tests every instruction method in instructions.cpp, plus clock().
//
// SETUP: chip8.h needs this line inside the class so the test can see private stuff:
//
//     friend struct Chip8Test;
//
// BUILD (from the project root, leaves out src/main.cpp):
//
//     g++ -std=c++17 -Wall -Iinclude tests/test_instructions.cpp src/chip8.cpp src/instructions.cpp -o test_instructions
//     ./test_instructions
//
// SIGNATURES THIS FILE EXPECTS (change the calls if yours are different):
//     push(uint16_t nnn)              pop()
//     jump(uint16_t nnn)              clearScreen()
//     jump_offset(uint16_t nnn)       wait_key(x)
//     clock()
//     skp_equals(x, nn)               skp_nequals(x, nn)
//     skp_iftrue(x, y)                skp_iffalse(x, y)
//     set_reg(x, nn)                  add_reg(x, nn)
//     set_to_reg(x, y)  logic_or/and/xor(x, y)  add/subtr/reverse_subtr(x, y)
//     rshift(x)                       lshift(x)
//     set_index(uint16_t nnn)         add_index(x)
//     font_char(x)                    decimal_conv(x)
//     cache_regs(x)                   fetch_regs(x)
//     random(x, nn)                   draw(x, y, n)
//     skip_if_key(x)                  skip_not_key(x)
//     get_dt(x)                       set_dt(x)        set_st(x)
//     setKey(key)                     clearKey(key)
//
// Registers are assumed to be named v. Font is assumed to start at 0x050.

#include "chip8.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <exception>

static int passed = 0;
static int failed = 0;

static void check(const std::string& name, int got, int expected) {
    if (got == expected) {
        std::cout << "  PASS  " << name << "\n";
        passed++;
    } else {
        std::cout << "  FAIL  " << name
                  << "  (expected 0x" << std::hex << expected
                  << ", got 0x" << got << std::dec << ")\n";
        failed++;
    }
}

static void section(const std::string& name) {
    std::cout << "\n" << name << "\n";
}

struct Chip8Test {

    // ---------------- Stack ----------------
    static void stack() {
        section("Stack: call and return");
        {
            Chip8 c;
            c.pc = 0x202;
            c.push(0x300);
            check("2NNN sets pc to NNN", c.pc, 0x300);
            check("2NNN saves full 16-bit pc (not 0x0)", c.stack.top(), 0x202);
            check("2NNN stack size is 1", c.stack.size(), 1);

            c.pop();
            check("00EE returns to saved pc", c.pc, 0x202);
            check("00EE stack is empty again", c.stack.size(), 0);
        }
        {
            Chip8 c;
            c.pc = 0x210;
            c.push(0x300);   // saves 0x210
            c.pc = 0x320;
            c.push(0x400);   // saves 0x320
            c.pop();
            check("Nested return goes to inner caller", c.pc, 0x320);
            c.pop();
            check("Nested return goes to outer caller", c.pc, 0x210);
        }

        section("Stack: error handling");
        {
            // Return with an empty stack
            Chip8 c;
            c.pc = 0x250;
            bool threw = false;
            try {
                c.pop();
            } catch (const std::exception& e) {
                threw = true;
                std::cout << "  INFO  empty pop threw: " << e.what() << "\n";
            } catch (...) {
                threw = true;
                std::cout << "  INFO  empty pop threw something\n";
            }
            bool handled = threw || c.pc == 0x250;
            check("00EE on empty stack is caught (throws or leaves pc alone)", handled, true);
        }
        {
            // More than 16 calls deep
            Chip8 c;
            c.pc = 0x200;
            bool threw = false;
            for (int n = 0; n < 16; n++) {
                c.push(0x300);
            }
            check("16 calls deep is allowed", c.stack.size(), 16);
            try {
                c.push(0x300);
            } catch (const std::exception& e) {
                threw = true;
                std::cout << "  INFO  17th call threw: " << e.what() << "\n";
            } catch (...) {
                threw = true;
                std::cout << "  INFO  17th call threw something\n";
            }
            bool handled = threw || c.stack.size() <= 16;
            check("17th call is caught (throws or stack stays at 16)", handled, true);
        }
    }

    // ---------------- Screen and flow ----------------
    static void screen_and_flow() {
        section("Screen and flow");
        Chip8 c;
        c.display[3][4] = true;
        c.display[63][31] = true;
        c.clearScreen();
        check("00E0 clears pixel (3,4)", c.display[3][4], false);
        check("00E0 clears pixel (63,31)", c.display[63][31], false);

        c.jump(0x456);
        check("1NNN sets pc", c.pc, 0x456);

        c.v.at(0) = 4;
        c.jump_offset(0x300);
        check("BNNN jumps to NNN + V0", c.pc, 0x304);

        c.v.at(0) = 0;
        c.v.at(3) = 9;
        c.jump_offset(0x300);
        check("BNNN uses V0, not VX (original CHIP-8)", c.pc, 0x300);
    }

    // ---------------- Skips ----------------
    static void skips() {
        section("Skips");
        Chip8 c;
        c.v.at(1) = 5; c.v.at(2) = 5; c.v.at(3) = 9;

        c.pc = 0x200; c.skp_equals(1, 5);
        check("3XNN skips when equal", c.pc, 0x202);
        c.pc = 0x200; c.skp_equals(1, 6);
        check("3XNN no skip when not equal", c.pc, 0x200);

        c.pc = 0x200; c.skp_nequals(1, 6);
        check("4XNN skips when not equal", c.pc, 0x202);
        c.pc = 0x200; c.skp_nequals(1, 5);
        check("4XNN no skip when equal", c.pc, 0x200);

        c.pc = 0x200; c.skp_iftrue(1, 2);
        check("5XY0 skips when equal", c.pc, 0x202);
        c.pc = 0x200; c.skp_iftrue(1, 3);
        check("5XY0 no skip when not equal", c.pc, 0x200);

        c.pc = 0x200; c.skp_iffalse(1, 3);
        check("9XY0 skips when not equal", c.pc, 0x202);
        c.pc = 0x200; c.skp_iffalse(1, 2);
        check("9XY0 no skip when equal", c.pc, 0x200);
    }

    // ---------------- Registers ----------------
    static void v() {
        section("Registers");
        Chip8 c;
        c.set_reg(4, 0x42);
        check("6XNN sets VX", c.v.at(4), 0x42);

        c.v.at(5) = 10;
        c.add_reg(5, 20);
        check("7XNN adds", c.v.at(5), 30);

        c.v.at(5) = 250; c.v.at(0xF) = 0x77;
        c.add_reg(5, 10);
        check("7XNN wraps 250+10", c.v.at(5), 4);
        check("7XNN leaves VF alone", c.v.at(0xF), 0x77);
    }

    // ---------------- Math and logic ----------------
    static void math_and_logic() {
        section("Math and logic");
        Chip8 c;

        c.v.at(1) = 0x12; c.v.at(2) = 0x34;
        c.set_to_reg(1, 2);
        check("8XY0 copies VY", c.v.at(1), 0x34);

        c.v.at(1) = 0b1100; c.v.at(2) = 0b1010;
        c.logic_or(1, 2);
        check("8XY1 OR", c.v.at(1), 0b1110);
        c.v.at(1) = 0b1100;
        c.logic_and(1, 2);
        check("8XY2 AND", c.v.at(1), 0b1000);
        c.v.at(1) = 0b1100;
        c.logic_xor(1, 2);
        check("8XY3 XOR", c.v.at(1), 0b0110);

        // 8XY4
        c.v.at(1) = 10; c.v.at(2) = 20; c.add(1, 2);
        check("8XY4 10+20 result", c.v.at(1), 30);
        check("8XY4 10+20 VF", c.v.at(0xF), 0);
        c.v.at(1) = 200; c.v.at(2) = 100; c.add(1, 2);
        check("8XY4 200+100 result", c.v.at(1), 44);
        check("8XY4 200+100 VF", c.v.at(0xF), 1);
        c.v.at(0xF) = 200; c.v.at(1) = 100; c.add(0xF, 1);
        check("8XY4 X=F, flag wins", c.v.at(0xF), 1);

        // 8XY5
        c.v.at(1) = 10; c.v.at(2) = 3; c.subtr(1, 2);
        check("8XY5 10-3 result", c.v.at(1), 7);
        check("8XY5 10-3 VF", c.v.at(0xF), 1);
        c.v.at(1) = 3; c.v.at(2) = 10; c.subtr(1, 2);
        check("8XY5 3-10 result", c.v.at(1), 249);
        check("8XY5 3-10 VF", c.v.at(0xF), 0);
        c.v.at(1) = 5; c.v.at(2) = 5; c.subtr(1, 2);
        check("8XY5 5-5 result", c.v.at(1), 0);
        check("8XY5 5-5 VF", c.v.at(0xF), 1);
        c.v.at(0xF) = 10; c.v.at(1) = 3; c.subtr(0xF, 1);
        check("8XY5 X=F, flag wins", c.v.at(0xF), 1);

        // 8XY7
        c.v.at(1) = 3; c.v.at(2) = 10; c.reverse_subtr(1, 2);
        check("8XY7 10-3 result", c.v.at(1), 7);
        check("8XY7 10-3 VF", c.v.at(0xF), 1);
        c.v.at(1) = 10; c.v.at(2) = 3; c.reverse_subtr(1, 2);
        check("8XY7 3-10 result", c.v.at(1), 249);
        check("8XY7 3-10 VF", c.v.at(0xF), 0);
        c.v.at(0xF) = 3; c.v.at(1) = 10; c.reverse_subtr(0xF, 1);
        check("8XY7 X=F, flag wins", c.v.at(0xF), 1);

        // 8XY6
        c.v.at(1) = 0b10110101; c.rshift(1);
        check("8XY6 result", c.v.at(1), 0b01011010);
        check("8XY6 VF when low bit 1", c.v.at(0xF), 1);
        c.v.at(1) = 0b10110100; c.rshift(1);
        check("8XY6 VF when low bit 0", c.v.at(0xF), 0);
        c.v.at(0xF) = 0b00000011; c.rshift(0xF);
        check("8XY6 X=F, flag wins", c.v.at(0xF), 1);

        // 8XYE
        c.v.at(1) = 0b10110101; c.lshift(1);
        check("8XYE result", c.v.at(1), 0b01101010);
        check("8XYE VF when high bit 1", c.v.at(0xF), 1);
        c.v.at(1) = 0b00110101; c.lshift(1);
        check("8XYE VF when high bit 0", c.v.at(0xF), 0);
        c.v.at(0xF) = 0b10000000; c.lshift(0xF);
        check("8XYE X=F, flag wins", c.v.at(0xF), 1);
    }

    // ---------------- Index and memory ----------------
    static void index_and_memory() {
        section("Index and memory");
        Chip8 c;

        c.set_index(0x123);
        check("ANNN sets I", c.i, 0x123);

        c.i = 0x100; c.v.at(2) = 0x10;
        c.add_index(2);
        check("FX1E adds VX to I", c.i, 0x110);

        c.v.at(1) = 0xA;
        c.font_char(1);
        check("FX29 points I at 'A'", c.i, 0x050 + 0xA * 5);
        c.v.at(1) = 0x1A;
        c.font_char(1);
        check("FX29 only uses low nibble", c.i, 0x050 + 0xA * 5);

        c.v.at(1) = 254; c.i = 0x500;
        c.decimal_conv(1);
        check("FX33 hundreds", c.memory.at(0x500), 2);
        check("FX33 tens", c.memory.at(0x501), 5);
        check("FX33 ones", c.memory.at(0x502), 4);
        check("FX33 leaves I alone", c.i, 0x500);

        c.v.at(1) = 7; c.i = 0x510;
        c.decimal_conv(1);
        check("FX33 small number hundreds", c.memory.at(0x510), 0);
        check("FX33 small number tens", c.memory.at(0x511), 0);
        check("FX33 small number ones", c.memory.at(0x512), 7);

        // FX55
        for (int j = 0; j < 16; j++) c.v.at(j) = 0;
        c.v.at(0) = 1; c.v.at(1) = 2; c.v.at(2) = 3; c.v.at(3) = 4; c.v.at(4) = 99;
        c.i = 0x300;
        c.cache_regs(3);
        check("FX55 stores V0", c.memory.at(0x300), 1);
        check("FX55 stores V3", c.memory.at(0x303), 4);
        check("FX55 stops at VX", c.memory.at(0x304), 0);
        std::cout << "  INFO  FX55 left I at 0x" << std::hex << c.i << std::dec
                  << " (0x300 or 0x304 depending on your quirk)\n";

        // FX65
        for (int j = 0; j < 16; j++) c.v.at(j) = 0;
        c.memory.at(0x400) = 0xA; c.memory.at(0x401) = 0xB;
        c.memory.at(0x402) = 0xC; c.memory.at(0x403) = 0xD;
        c.i = 0x400;
        c.fetch_regs(2);
        check("FX65 loads V0", c.v.at(0), 0xA);
        check("FX65 loads V2", c.v.at(2), 0xC);
        check("FX65 stops at VX", c.v.at(3), 0);
        std::cout << "  INFO  FX65 left I at 0x" << std::hex << c.i << std::dec
                  << " (0x400 or 0x403 depending on your quirk)\n";
    }

    // ---------------- Random ----------------
    static void random() {
        section("Random");
        Chip8 c;
        bool in_mask = true;
        bool saw_different = false;
        int first = -1;
        for (int t = 0; t < 200; t++) {
            c.random(6, 0x0F);
            int val = c.v.at(6);
            if (val > 0x0F) in_mask = false;
            if (first == -1) first = val;
            else if (val != first) saw_different = true;
        }
        check("CXNN result stays inside NN mask", in_mask, true);
        check("CXNN actually varies", saw_different, true);

        c.random(6, 0x00);
        check("CXNN with NN=0 always gives 0", c.v.at(6), 0);
    }

    // ---------------- Draw ----------------
    static void draw() {
        section("Draw");
        {
            // Font "0" at (0,0): F0 90 90 90 F0
            Chip8 c;
            c.i = 0x050;
            c.v.at(0) = 0; c.v.at(1) = 0;
            c.draw(0, 1, 5);
            check("DXYN row 0 pixel 0 on", c.display[0][0], true);
            check("DXYN row 0 pixel 3 on", c.display[3][0], true);
            check("DXYN row 0 pixel 4 off", c.display[4][0], false);
            check("DXYN row 1 pixel 0 on", c.display[0][1], true);
            check("DXYN row 1 pixel 1 off", c.display[1][1], false);
            check("DXYN row 1 pixel 3 on", c.display[3][1], true);
            check("DXYN row 4 pixel 2 on", c.display[2][4], true);
            check("DXYN row 5 not drawn", c.display[0][5], false);
            check("DXYN no collision sets VF 0", c.v.at(0xF), 0);

            c.draw(0, 1, 5);
            check("DXYN drawing again erases it", c.display[0][0], false);
            check("DXYN collision sets VF 1", c.v.at(0xF), 1);
        }
        {
            // Position from VX and VY, not X and Y
            Chip8 c;
            c.i = 0x050;
            c.v.at(2) = 10; c.v.at(3) = 5;
            c.draw(2, 3, 1);
            check("DXYN uses VX and VY for position", c.display[10][5], true);
        }
        {
            // Start position wraps
            Chip8 c;
            c.i = 0x050;
            c.v.at(0) = 66; c.v.at(1) = 33;   // wraps to (2, 1)
            c.draw(0, 1, 1);
            check("DXYN start x and y wrap", c.display[2][1], true);
        }
        {
            // Right edge clips
            Chip8 c;
            c.i = 0x050;
            c.v.at(0) = 62; c.v.at(1) = 0;
            c.draw(0, 1, 1);                   // F0 = 4 pixels wide
            check("DXYN pixel at x=62 on", c.display[62][0], true);
            check("DXYN pixel at x=63 on", c.display[63][0], true);
            check("DXYN right edge clips (x=0 stays off)", c.display[0][0], false);
        }
        {
            // Bottom edge clips
            Chip8 c;
            c.i = 0x050;
            c.v.at(0) = 0; c.v.at(1) = 30;
            c.draw(0, 1, 5);
            check("DXYN pixel at y=30 on", c.display[0][30], true);
            check("DXYN pixel at y=31 on", c.display[0][31], true);
            check("DXYN bottom edge clips (y=0 stays off)", c.display[0][0], false);
        }
        {
            // I does not move
            Chip8 c;
            c.i = 0x050;
            c.draw(0, 1, 5);
            check("DXYN leaves I alone", c.i, 0x050);
        }
    }

    // ---------------- Keys ----------------
    static void keys() {
        section("Keys");
        Chip8 c;
        c.v.at(1) = 0xA;

        c.clearKey(0xA);
        c.pc = 0x200; c.skip_if_key(1);
        check("EX9E no skip when key up", c.pc, 0x200);
        c.pc = 0x200; c.skip_not_key(1);
        check("EXA1 skips when key up", c.pc, 0x202);

        c.setKey(0xA);
        c.pc = 0x200; c.skip_if_key(1);
        check("EX9E skips when key down", c.pc, 0x202);
        c.pc = 0x200; c.skip_not_key(1);
        check("EXA1 no skip when key down", c.pc, 0x200);

        c.setKey(0);
        check("setKey works for key 0", c.key_state[0], true);
        c.setKey(0xF);
        check("setKey works for key F", c.key_state[0xF], true);
    }

    // ---------------- Wait for key ----------------
    static void wait_key() {
        section("Wait for key (FX0A)");
        Chip8 c;
        c.v.at(3) = 0xEE; // junk so we can tell if it got written

        // No key down: should back pc up and wait
        c.pc = 0x202;
        c.wait_key(3);
        check("FX0A no key: pc moves back 2", c.pc, 0x200);
        check("FX0A no key: VX untouched", c.v.at(3), 0xEE);

        // Key 5 goes down: still waiting for release
        c.setKey(5);
        c.pc = 0x202;
        c.wait_key(3);
        check("FX0A key down: still waiting", c.pc, 0x200);
        check("FX0A key down: VX not set yet", c.v.at(3), 0xEE);

        // Still held: keep waiting
        c.pc = 0x202;
        c.wait_key(3);
        check("FX0A key held: still waiting", c.pc, 0x200);

        // Released: store it and move on
        c.clearKey(5);
        c.pc = 0x202;
        c.wait_key(3);
        check("FX0A key released: pc moves on", c.pc, 0x202);
        check("FX0A key released: VX holds the key", c.v.at(3), 5);

        // Next FX0A starts fresh
        c.pc = 0x202;
        c.wait_key(3);
        check("FX0A resets after finishing", c.pc, 0x200);
    }

    // ---------------- Clock ----------------
    static void clock() {
        section("Clock");
        Chip8 c;
        c.delay_timer = 2; c.sound_timer = 1;
        c.clock();
        check("clock counts delay timer down", c.delay_timer, 1);
        check("clock counts sound timer down", c.sound_timer, 0);
        c.clock();
        check("clock delay timer reaches 0", c.delay_timer, 0);
        check("clock sound timer stays at 0", c.sound_timer, 0);
        c.clock();
        check("clock delay timer doesn't wrap", c.delay_timer, 0);
    }

    // ---------------- Timers ----------------
    static void timers() {
        section("Timers");
        Chip8 c;

        c.v.at(3) = 60; c.set_dt(3);
        check("FX15 sets delay timer", c.delay_timer, 60);

        c.v.at(4) = 30; c.set_st(4);
        check("FX18 sets sound timer", c.sound_timer, 30);

        c.delay_timer = 42;
        c.get_dt(5);
        check("FX07 copies delay timer into VX", c.v.at(5), 42);
    }
};

int main() {
    Chip8Test::stack();
    Chip8Test::screen_and_flow();
    Chip8Test::skips();
    Chip8Test::v();
    Chip8Test::math_and_logic();
    Chip8Test::index_and_memory();
    Chip8Test::random();
    Chip8Test::draw();
    Chip8Test::keys();
    Chip8Test::wait_key();
    Chip8Test::timers();
    Chip8Test::clock();

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}