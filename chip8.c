#include "chip8.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL2/SDL.h"

void print_instruction(instruction i) {
  fprintf(stderr, "opcode: %d hi: %2x lo: %2x\n", i.opcode, i.hi, i.lo);
}

void load_hex_fonts(chip8* system) {
  // Format for hex font characters is taken from
  // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.4

  // Ascii "0".
  memcpy(system->memory, (char[]){0xF0, 0x90, 0x90, 0x90, 0xF0}, 5);
  // Ascii "1".
  memcpy(system->memory + 5, (char[]){0x20, 0x60, 0x20, 0x20, 0x70}, 5);
  // Ascii "2".
  memcpy(system->memory + 10, (char[]){0xF0, 0x10, 0xF0, 0x80, 0xF0}, 5);
  // Ascii "3".
  memcpy(system->memory + 15, (char[]){0xF0, 0x10, 0xF0, 0x10, 0xF0}, 5);
  // Ascii "4".
  memcpy(system->memory + 20, (char[]){0x90, 0x90, 0xF0, 0x10, 0x10}, 5);
  // Ascii "5".
  memcpy(system->memory + 25, (char[]){0xF0, 0x80, 0xF0, 0x10, 0xF0}, 5);
  // Ascii "6".
  memcpy(system->memory + 30, (char[]){0xF0, 0x80, 0xF0, 0x90, 0xF0}, 5);
  // Ascii "7".
  memcpy(system->memory + 35, (char[]){0xF0, 0x10, 0x20, 0x40, 0x40}, 5);
  // Ascii "8".
  memcpy(system->memory + 40, (char[]){0xF0, 0x90, 0xF0, 0x90, 0xF0}, 5);
  // Ascii "9".
  memcpy(system->memory + 45, (char[]){0xF0, 0x90, 0xF0, 0x10, 0xF0}, 5);
  // Ascii "A".
  memcpy(system->memory + 50, (char[]){0xF0, 0x90, 0xF0, 0x90, 0x90}, 5);
  // Ascii "B".
  memcpy(system->memory + 55, (char[]){0xE0, 0x90, 0xE0, 0x90, 0xE0}, 5);
  // Ascii "C".
  memcpy(system->memory + 60, (char[]){0xF0, 0x80, 0x80, 0x80, 0xF0}, 5);
  // Ascii "D".
  memcpy(system->memory + 65, (char[]){0xE0, 0x90, 0x90, 0x90, 0xE0}, 5);
  // Ascii "E".
  memcpy(system->memory + 70, (char[]){0xF0, 0x80, 0xF0, 0x80, 0xF0}, 5);
  // Ascii "F".
  memcpy(system->memory + 75, (char[]){0xF0, 0x80, 0xF0, 0x80, 0x80}, 5);
}

int load_program(const char* filename, chip8* system) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  uint64_t length = ftell(f);
  if (length > sizeof(system->memory)) {
    fprintf(stderr, "File is to large to fit in chip8 system memory.\n");
    return 1;
  }

  fseek(f, 0, SEEK_SET);
  fread(system->memory + 0x200, 1, length, f);

  fclose(f);
  return 0;
}

chip8 initialize_chip8() {
  chip8 system;
  // Zero-out all of the values in |system|.
  memset(&system, 0, sizeof(system));

  return system;
}

void print_chip8(chip8 system) {
  for (int i = 0; i < 16; ++i) {
    fprintf(stderr, "V%d: %d, ", i, system.V[i]);
  }
  fprintf(stderr, "\n");
}

instruction get_instruction(const chip8* system) {
  // Read the next 2 bytes from memory.
  instruction next;
  next.hi = system->memory[system->pc];
  next.lo = system->memory[system->pc + 1];

  // The highest 4 bits of |hi| are used to determine the opcode.
  uint8_t msb = next.hi >> 4;

  // The last 4 bits of |lo| are used to disambiguate instructions with the same
  // value for |msb|.
  uint8_t lsb = next.lo & 0x0F;

  switch (msb) {
    case 0x0:
      if (next.hi == 0x00) {
        if (next.lo == 0xE0) {
          next.opcode = CLEAR_SCREEN;
        } else if (next.lo == 0xEE) {
          next.opcode = RETURN;
        } else {
          fprintf(stderr, "Unexpected lo:%2x for msb 0x00\n", next.lo);
          exit(1);
        }
      } else {
        fprintf(stderr, "machine code routine...\n");
        exit(1);
        next.opcode = 0;
      }
      break;
    case 0x01:
      next.opcode = JUMP;
      break;
    case 0x02:
      next.opcode = CALL;
      break;
    case 0x03:
      next.opcode = IF_X_EQ_NN;
      break;
    case 0x04:
      next.opcode = IF_X_NEQ_NN;
      break;
    case 0x05:
      next.opcode = IF_X_EQ_Y;
      break;
    case 0x06:
      next.opcode = SET_X_NN;
      break;
    case 0x07:
      next.opcode = ADD_X_NN;
      break;
    case 0x08:
      switch (lsb) {
        case 0:
          next.opcode = SET_X_Y;
          break;
        case 1:
          next.opcode = OR_X_Y;
          break;
        case 2:
          next.opcode = AND_X_Y;
          break;
        case 3:
          next.opcode = XOR_X_Y;
          break;
        case 4:
          next.opcode = ADD_X_Y;
          break;
        case 5:
          next.opcode = SUB_X_Y;
          break;
        case 6:
          next.opcode = SHIFT_X_RIGHT;
          break;
        case 7:
          next.opcode = SUB_X_Y_REV;
          break;
        case 0x0E:
          next.opcode = SHIFT_X_LEFT;
          break;
        default:
          fprintf(stderr, "Unexpected lsb:%2x for msb 0x08\n", lsb);
          exit(1);
          break;
      }
      break;
    case 0x09:
      next.opcode = IF_X_NEQ_Y;
      break;
    case 0x0A:
      next.opcode = SET_I_NNN;
      break;
    case 0x0B:
      next.opcode = JUMP_ADDR;
      break;
    case 0x0C:
      next.opcode = SET_RAND;
      break;
    case 0x0D:
      next.opcode = DRAW;
      break;
    case 0x0E:
      switch (next.lo) {
        case 0x9E:
          next.opcode = IF_KEY_EQ;
          break;
        case 0xA1:
          next.opcode = IF_KEY_NEQ;
          break;
        default:
          fprintf(stderr, "Unexpected lo:%2x for msb 0x0E\n", next.lo);
          exit(1);
          break;
      }
      break;
    case 0x0F:
      switch (next.lo) {
        case 0x07:
          next.opcode = GET_DELAY;
          break;
        case 0x0A:
          next.opcode = GET_KEY;
          break;
        case 0x15:
          next.opcode = SET_DELAY;
          break;
        case 0x18:
          next.opcode = SET_SOUND;
          break;
        case 0x1E:
          next.opcode = ADD_X_I;
          break;
        case 0x29:
          next.opcode = LOAD_CHAR;
          break;
        case 0x33:
          next.opcode = BCD;
          break;
        case 0x55:
          next.opcode = REG_DUMP;
          break;
        case 0x65:
          next.opcode = REG_LOAD;
          break;
        default:
          fprintf(stderr, "Unexpected lo:%2x for msb 0x0F\n", next.lo);
          exit(1);
          break;
      }
      break;
    default:
      fprintf(stderr, "Unexpected value for msb:%2x\n", msb);
      exit(1);
      break;
  }

  return next;
}

void set_register(uint8_t vx, uint8_t val, chip8* system) {
  system->V[vx] = val;
}

void clear_screen(instruction next, chip8* system) {
  memset(system->screen, 0, sizeof(system->screen));
}

void return_subroutine(instruction next, chip8* system) {
  // TODO: add a check that sp > 0.
  // Retrieve the old value of |pc| from the stack.
  system->sp -= 1;
  system->pc = system->stack[system->sp];
}

void jump(instruction next, chip8* system) {
  // Extract the value of NNN.
  uint16_t n = ((next.hi & 0x0F) << 8) + next.lo;
  system->pc = n;

  // Mark that a jump operation has been performed.
  system->jumped = 1;
}

void call(instruction next, chip8* system) {
  // Extract the value of NNN.
  uint16_t n = ((next.hi & 0x0F) << 8) + next.lo;
  // Store the current program counter on the stack.
  system->stack[system->sp] = system->pc;
  system->sp += 1;

  // Jump to the location of |n|.
  system->pc = n;

  // Mark that a jump operation has been performed.
  system->jumped = 1;
}

void if_x_eq_nn(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  uint8_t n = next.lo;
  system->skip = (system->V[x] == n);
}

void if_x_neq_nn(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  uint8_t n = next.lo;
  system->skip = (system->V[x] != n);
}

void if_x_eq_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->skip = (system->V[x] == system->V[y]);
}

void set_x_nn(instruction next, chip8* system) {
  // Extract value of X.
  uint8_t x = next.hi & 0x0F;
  uint8_t n = next.lo;
  system->V[x] = n;
}

void add_x_nn(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  uint8_t n = next.lo;
  system->V[x] += n;
}

void set_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->V[x] = system->V[y];
}

void or_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->V[x] = system->V[x] | system->V[y];
}

void and_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->V[x] = system->V[x] & system->V[y];
}

void xor_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->V[x] = system->V[x] ^ system->V[y];
}

void add_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;

  // If the result of the addition would cause an overflow set the carry flag
  // (VF) to 1.
  uint16_t sum = system->V[x] + system->V[y];
  system->V[0xF] = 0;
  if (sum > 255) {
    system->V[0xF] = 1;
  }

  system->V[x] = system->V[x] + system->V[y];
}

void sub_x_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;

  int16_t sub = system->V[x] - system->V[y];
  system->V[0xF] = 1;
  if (sub < 0) {
    system->V[0xF] = 0;
  }

  system->V[x] = system->V[x] - system->V[y];
}

void shift_x_right(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  // Store least significant bit of VX in VF.
  system->V[0xF] = system->V[x] & 0b00000001;

  // Shift VX to the right.
  system->V[x] = system->V[x] >> 1;
}

void sub_x_y_rev(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;

  uint16_t sub = system->V[y] - system->V[x];
  system->V[0xF] = 1;
  if (sub < 0) {
    system->V[0xF] = 0;
  }

  system->V[x] = system->V[y] - system->V[x];
}

void shift_x_left(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  // Store most significant bit of VX in VF.
  system->V[0xF] = system->V[x] >> 7;

  // Shift VX to the left.
  system->V[x] = system->V[x] << 1;
}

void if_x_neq_y(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  system->skip = (system->V[x] != system->V[y]);
}

void set_i_nnn(instruction next, chip8* system) {
  // Extract the value of NNN.
  uint16_t n = ((next.hi & 0x0F) << 8) + next.lo;
  system->I = n;
}

void jump_addr(instruction next, chip8* system) {
  // Extract the value of NNN.
  uint16_t n = ((next.hi & 0x0F) << 8) + next.lo;
  // Jump to V0 + n.
  system->pc = system->V[0] + n;

  // Mark that a jump operation has been performed.
  system->jumped = 1;
}

void set_rand(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  uint8_t n = next.lo;
  // Generate the random number.
  uint8_t r = rand() % 256;

  system->V[x] = r & n;
}

void draw_row(uint8_t x, uint8_t y, uint8_t row, chip8* system) {
  for (int i = 0; i < 8; ++i) {
    uint8_t sprite_pixel = (row >> i) & 1;
    // Start from the right side of the row.
    uint16_t screen_index = (x + (7 - i)) + (64 * y);

    // Store the previous state of the screen pixel.
    uint8_t previous_state = system->screen[screen_index];

    // Draw the pixel to the screen.
    system->screen[screen_index] = system->screen[screen_index] ^ sprite_pixel;

    // If a previously set pixel was flipped, set VF = 1.
    if (previous_state == 1 && system->screen[screen_index] == 0) {
      system->V[0x0F] = 1;
    }
  }
}

void draw(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  // Extract the value of Y;
  uint8_t y = next.lo >> 4;
  // Extract the value of N.
  uint8_t n = next.lo & 0x0F;

  // Set the initial state of VF to 0.
  system->V[0x0F] = 0;

  for (int i = 0; i < n; ++i) {
    // Load the row as bit-encoded memory.
    uint8_t row = system->memory[system->I + i];
    draw_row(system->V[x], system->V[y] + i, row, system);
  }

  system->draw_flag = 1;
}

void if_key_eq(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->skip = system->keys[system->V[x]];
}

void if_key_neq(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->skip = !system->keys[system->V[x]];
}

void get_delay(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->V[x] = system->delay_timer;
}

uint8_t is_chip8_key(SDL_Keycode keycode) {
  switch (keycode) {
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_q:
    case SDLK_w:
    case SDLK_e:
    case SDLK_r:
    case SDLK_a:
    case SDLK_s:
    case SDLK_d:
    case SDLK_f:
    case SDLK_z:
    case SDLK_x:
    case SDLK_c:
    case SDLK_v:
      return 1;
    default:
      return 0;
  }
  return 0;
}

uint8_t hex_keycode(SDL_Keycode keycode) {
  switch (keycode) {
    case SDLK_1:
      return 0x01;
    case SDLK_2:
      return 0x02;
    case SDLK_3:
      return 0x03;
    case SDLK_4:
      return 0x0C;
    case SDLK_q:
      return 0x04;
    case SDLK_w:
      return 0x05;
    case SDLK_e:
      return 0x06;
    case SDLK_r:
      return 0x0D;
    case SDLK_a:
      return 0x07;
    case SDLK_s:
      return 0x08;
    case SDLK_d:
      return 0x09;
    case SDLK_f:
      return 0x0E;
    case SDLK_z:
      return 0x0A;
    case SDLK_x:
      return 0x00;
    case SDLK_c:
      return 0x0B;
    case SDLK_v:
      return 0x0F;
    default:
      fprintf(stderr, "Invalid hex keycode %d\n", keycode);
      return 0;
  }
}

SDL_Event wait_for_keypress() {
  SDL_Event e;
  while (SDL_PollEvent(&e) == 0 ||
         ((e.type != SDL_KEYDOWN || !is_chip8_key(e.key.keysym.sym)) &&
          e.type != SDL_QUIT))
    ;
  return e;
}

void get_key(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  SDL_Event e = wait_for_keypress();
  if (e.type == SDL_QUIT) {
    fprintf(stderr, "Received quit event");
    exit(1);
  }
  if (e.type == SDL_KEYDOWN) {
    SDL_Keycode keycode = e.key.keysym.sym;
    fprintf(stderr, "key pressed: %d\n", keycode);
    // TODO: Add validation for chip8 keycode values.
    // Convert pressed key to chip8 hex keyboard value.

    // Store pressed key in VX
    system->V[x] = hex_keycode(keycode);
  }
}

void set_delay(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->delay_timer = system->V[x];
}

void set_sound(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->sound_timer = system->V[x];
}

void add_x_i(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;
  system->I += system->V[x];
}

void load_char(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  switch (system->V[x]) {
    case 0x00:
      system->I = 0x00;  // 0
      break;
    case 0x01:
      system->I = 5;  // 5
      break;
    case 0x02:
      system->I = 10;  // 10
      break;
    case 0x03:
      system->I = 15;  // 15
      break;
    case 0x04:
      system->I = 20;  // 20
      break;
    case 0x05:
      system->I = 25;  // 25
      break;
    case 0x06:
      system->I = 30;  // 30
      break;
    case 0x07:
      system->I = 35;  // 35
      break;
    case 0x08:
      system->I = 40;  // 40
      break;
    case 0x09:
      system->I = 45;  // 45
      break;
    case 0x0A:
      system->I = 50;  // 50
      break;
    case 0x0B:
      system->I = 55;  // 55
      break;
    case 0x0C:
      system->I = 60;  // 60
      break;
    case 0x0D:
      system->I = 65;  // 65
      break;
    case 0x0E:
      system->I = 70;  // 70
      break;
    case 0x0F:
      system->I = 75;  // 75
      break;
    default:
      break;
  }
}

void bcd(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  uint8_t vx = system->V[x];
  // Store the ones digit at I+2
  system->memory[system->I + 2] = vx % 10;

  // Store the tens digit at I+1
  vx /= 10;
  system->memory[system->I + 1] = vx % 10;

  // Store the hundreds digit at I
  vx /= 10;
  system->memory[system->I] = vx % 10;
}

void reg_dump(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  // Store the values of each register from V0 to VX in memory.
  for (uint8_t i = 0; i <= x; ++i) {
    system->memory[system->I + i] = system->V[i];
  }
}

void reg_load(instruction next, chip8* system) {
  // Extract the value of X.
  uint8_t x = next.hi & 0x0F;

  // Load values for V0 to VX from memory.
  for (uint8_t i = 0; i <= x; ++i) {
    system->V[i] = system->memory[system->I + i];
  }
}

void perform_instruction(instruction next, chip8* system) {
  switch (next.opcode) {
    case CLEAR_SCREEN:
      clear_screen(next, system);
      break;
    case RETURN:
      return_subroutine(next, system);
      break;
    case JUMP:
      jump(next, system);
      break;
    case CALL:
      call(next, system);
      break;
    case IF_X_EQ_NN:
      if_x_eq_nn(next, system);
      break;
    case IF_X_NEQ_NN:
      if_x_neq_nn(next, system);
      break;
    case IF_X_EQ_Y:
      if_x_eq_y(next, system);
      break;
    case SET_X_NN:
      set_x_nn(next, system);
      break;
    case ADD_X_NN:
      add_x_nn(next, system);
      break;
    case SET_X_Y:
      set_x_y(next, system);
      break;
    case OR_X_Y:
      or_x_y(next, system);
      break;
    case AND_X_Y:
      and_x_y(next, system);
      break;
    case XOR_X_Y:
      xor_x_y(next, system);
      break;
    case ADD_X_Y:
      add_x_y(next, system);
      break;
    case SUB_X_Y:
      sub_x_y(next, system);
      break;
    case SHIFT_X_RIGHT:
      shift_x_right(next, system);
      break;
    case SHIFT_X_LEFT:
      shift_x_left(next, system);
      break;
    case IF_X_NEQ_Y:
      if_x_neq_y(next, system);
      break;
    case SET_I_NNN:
      set_i_nnn(next, system);
      break;
    case JUMP_ADDR:
      jump_addr(next, system);
      break;
    case SET_RAND:
      set_rand(next, system);
      break;
    case DRAW:
      draw(next, system);
      break;
    case IF_KEY_EQ:
      if_key_eq(next, system);
      break;
    case IF_KEY_NEQ:
      if_key_neq(next, system);
      break;
    case GET_DELAY:
      get_delay(next, system);
      break;
    case GET_KEY:
      get_key(next, system);
      break;
    case SET_DELAY:
      set_delay(next, system);
      break;
    case SET_SOUND:
      set_sound(next, system);
      break;
    case ADD_X_I:
      add_x_i(next, system);
      break;
    case LOAD_CHAR:
      load_char(next, system);
      break;
    case BCD:
      bcd(next, system);
      break;
    case REG_DUMP:
      reg_dump(next, system);
      break;
    case REG_LOAD:
      reg_load(next, system);
      break;
    default:
      fprintf(stderr, "Unknown instruction: %d\n", next.opcode);
      break;
  }
}

void emulate_cycle(chip8* system) {
  // Determine next instrution.
  instruction next = get_instruction(system);

  // If the skip flag is set from the previous cycle, ignore the current
  // instruction and reset the skip flag.
  if (system->skip) {
    system->skip = 0;
  } else {
    perform_instruction(next, system);
  }

  // Increment the Program Counter by 2 bytes.
  if (!system->jumped) {
    system->pc += 2;
  } else {
    system->jumped = 0;
  }

  ++system->cycle;

  // Update system timers.
  if (system->cycle > CLOCK_SPEED / 60) {
    if (system->delay_timer > 0) {
      --system->delay_timer;
    }
    if (system->sound_timer > 0) {
      fprintf(stderr, "BEEP!\n");
      --system->sound_timer;
    }
  }
}