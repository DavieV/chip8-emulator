#define SDL_MAIN_HANDLED
#include <stdio.h>

#include "SDL2/SDL.h"
#include "chip8.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

void draw_screen(chip8 system, SDL_Surface *screen_surface) {
  // Fill the screen black.
  SDL_FillRect(screen_surface, NULL,
               SDL_MapRGB(screen_surface->format, 0, 0, 0));

  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 64; ++j) {
      // Skip drawing if the pixel in the screen isn't set.
      if (!system.screen[i * 64 + j]) {
        continue;
      }
      SDL_Rect rect;
      rect.y = i * 10;
      rect.x = j * 10;
      rect.h = 10;
      rect.w = 10;
      SDL_FillRect(screen_surface, &rect,
                   SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
    }
  }
}

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

  // load game into memory.
  if (load_program("test_opcode.ch8", &system)) {
    fprintf(stderr, "Failed to load program\n");
  }

  // Chip 8 game loop.
  uint8_t keep_window_open = 1;
  SDL_Event window_event;
  while (keep_window_open) {
    while (SDL_PollEvent(&window_event) > 0) {
      switch (window_event.type) {
        case SDL_QUIT:
          keep_window_open = 0;
          break;
      }
    }

    // Emulate one cycle.
    emulate_cycle(&system);

    // If the draw flag is set, update the screen.
    if (system.draw_flag) {
      draw_screen(system, screen_surface);
      SDL_UpdateWindowSurface(window);
      system.draw_flag = 0;
    }

    // Store key press state (Press and Release).
  }

  // Quit SDL
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}