#include "chip8.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdexcept>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCALE = 10;

int main( int argc, char* args[] ){
    
    SDL_Window* window = NULL; //window itself
    SDL_Renderer* renderer = NULL; //renderer
    SDL_Surface* screenSurface = NULL; //surface in the window

    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0 ){
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
     else{
        //Create window
        window = SDL_CreateWindow("CHIP8 EMULATOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

        if(window == NULL){
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else{
            //Get window surface
            screenSurface = SDL_GetWindowSurface(window);
            //Fill the surface white
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            //Update the surface
            SDL_UpdateWindowSurface(window);
            //Hack to get window to stay up
            SDL_Event e;
            bool quit = false;
            
            while(quit == false){ 
                while(SDL_PollEvent(&e)){ if(e.type == SDL_QUIT) quit = true; }
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);

            }
        }
}
}