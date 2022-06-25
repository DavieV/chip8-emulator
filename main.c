#define SDL_MAIN_HANDLED
#include <stdio.h>

#include <SDL2/SDL.h>

#include "chip8.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

int main(int argc, char *args[]) {
  // Start SDL
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_Window *window = SDL_CreateWindow("hello_sdl2", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Surface *screen_surface = SDL_GetWindowSurface(window);
  if (screen_surface == NULL) {
    fprintf(stderr, "could not create window surface: %s\n", SDL_GetError());
    return 1;
  }

  SDL_FillRect(screen_surface, NULL,
               SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window);

  // initialize chip8 system
  chip8 system = initialize_chip8();
  load_hex_fonts(&system);
  system.pc = 0x200;
  system.window = window;
  system.screen_surface = screen_surface;

  // load game into memory.
  if (load_program("pong.ch8", &system)) {
    fprintf(stderr, "Failed to load program\n");
    return 1;
  }

  // Chip 8 game loop.
  game_loop(&system);

  // Quit SDL
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}