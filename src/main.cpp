#include "chip8.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <exception>

//settings
constexpr int DISPLAY_W = 64;
constexpr int DISPLAY_H = 32;
constexpr int SCALE = 10;

constexpr int INSTRUCTIONS_PER_FRAME = 11;   // 700 per second
constexpr Uint32 FRAME_MS = 1000 / 60;       //about 16 ms, 60 frames per second

//sound
constexpr int SAMPLE_RATE = 44100;           //samples per second
constexpr int TONE_HZ = 440;                 //pitch of the beep
constexpr int16_t VOLUME = 3000;             //how loud, max is 32767

//SDL calls this whenever it needs more audio. Fills the buffer with a square wave.
void audioCallback(void* userdata, Uint8* stream, int len) {
    int* phase = static_cast<int*>(userdata);
    int16_t* samples = reinterpret_cast<int16_t*>(stream);
    int count = len / sizeof(int16_t);
    int halfPeriod = SAMPLE_RATE / TONE_HZ / 2;

    for (int s = 0; s < count; s++) {
        samples[s] = ((*phase / halfPeriod) % 2) ? VOLUME : -VOLUME;
        (*phase)++;
    }
}

//keyboard
// Real keyboard        CHIP-8 keypad
// 1 2 3 4         ->   1 2 3 C
// Q W E R         ->   4 5 6 D
// A S D F         ->   7 8 9 E
// Z X C V         ->   A 0 B F
// Returns the CHIP-8 key (0 to F), or -1 if it's not a CHIP-8 key.
int mapKey(SDL_Scancode sc) {
    switch (sc) {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
        default: return -1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom.ch8>\n";
        return 1;
    }

    //SDL setup
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("CHIP-8",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_W * SCALE, DISPLAY_H * SCALE, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == nullptr) {
        std::cerr << "Renderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //audio setup, keeps running without sound if it fails
    int phase = 0;
    SDL_AudioSpec want = {};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = &phase;

    SDL_AudioDeviceID audio = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (audio == 0) {
        std::cerr << "Audio failed (running without sound): " << SDL_GetError() << "\n";
    }
    int exitCode = 0;

    try {
        //emulator setup
        Chip8 chip;
        chip.load(argv[1]);

        //main loop
        bool running = true;
        SDL_Event e;

        while (running) {
            Uint32 frameStart = SDL_GetTicks();

            //events
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }
                else if (e.type == SDL_KEYDOWN && !e.key.repeat) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = false;
                    int key = mapKey(e.key.keysym.scancode);
                    if (key != -1) chip.setKey(key);
                }
                else if (e.type == SDL_KEYUP) {
                    int key = mapKey(e.key.keysym.scancode);
                    if (key != -1) chip.clearKey(key);
                }
            }

            //CPU
            for (int n = 0; n < INSTRUCTIONS_PER_FRAME; n++) {
                chip.exec();
            }

            //timers
            chip.clock();

            //sound, 0 means play and 1 means pause
            if (audio != 0) {
                SDL_PauseAudioDevice(audio, chip.play_sound() ? 0 : 1);
            }

            //draw
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int y = 0; y < DISPLAY_H; y++) {
                for (int x = 0; x < DISPLAY_W; x++) {
                    if (chip.getBit(x, y)) {
                        SDL_Rect pixel = { x * SCALE, y * SCALE, SCALE, SCALE };
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }
            SDL_RenderPresent(renderer);

            //wait out the rest of the frame
            Uint32 elapsed = SDL_GetTicks() - frameStart;
            if (elapsed < FRAME_MS) {
                SDL_Delay(FRAME_MS - elapsed);
            }
        }
    }
    catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << "\n";
        exitCode = 1;
    }

    //cleanup
    if (audio != 0) SDL_CloseAudioDevice(audio);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return exitCode;
}