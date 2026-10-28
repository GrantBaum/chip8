# CHIP-8 Emulator

A CHIP-8 emulator I'm building from scratch in C++ with SDL2.

## What is CHIP-8

CHIP-8 is a tiny virtual machine from the 1970s. People used it to make simple games on early home computers. It has 4KB of memory, 16 registers and a 64x32 black and white screen. That's about it. An emulator is a program that pretends to be that machine. It reads the game's raw bytes one instruction at a time and does what each one says.

## Why I'm doing this

I want to get into systems programming. I want to know what's actually happening at the hardware level. Memory, registers, bits and bytes. CHIP-8 is the classic first emulator. It's small enough to finish but real enough to teach you how a CPU works. I'm writing this myself. No copying someone else's emulator. I'm working from the docs and figuring it out as I go so there may be some unique quirks in here. It is not meant to be an emulator you trust, just one I am using to learn and hopefully will work by the end of it.

## Status

Work in progress.

- [x] Class layout
- [x] ROM loading
- [x] Font
- [x] Fetch
- [x] Most instructions
- [ ] Decode
- [ ] Drawing
- [ ] Main loop and timing
- [ ] Input
- [ ] Sound
- [ ] Play a real game

## Building

You need g++ and SDL2. On Ubuntu or WSL:

```
sudo apt install build-essential libsdl2-dev
```

Then:

```
make
./chip8
```

## Files

| File | What it does |
|------|--------------|
| `chip8.h` | The Chip8 class |
| `chip8.cpp` | Setup, loading and the fetch loop |
| `instructions.cpp` | One method per instruction |
| `font.h` | Built-in font sprites |
| `main.cpp` | Window, input and main loop |

## Resources

- [Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) by Tobias V. Langhoff
- [CHIP-8 test suite](https://github.com/Timendus/chip8-test-suite) by Timendus