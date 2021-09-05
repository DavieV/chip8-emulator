#include "chip8.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  // The first 4 bits of |hi| are used to determine the opcode.
  uint8_t msb = next.hi >> 4;

  switch (msb) {
    case 0x0:
      break;
    case 0x1:
      next.opcode = JUMP;
      break;
    case 0x2:
      next.opcode = CALL;
      break;
    case 0x3:
      next.opcode = IF_X_EQ_NN;
      break;
    case 0x4:
      next.opcode = IF_X_NEQ_NN;
      break;
    case 0x5:
      next.opcode = IF_X_EQ_Y;
      break;
    case 0x6:
      next.opcode = SET_X_NN;
      break;
    case 0x7:
      next.opcode = ADD_X_NN;
      break;
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    default:
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
}

void call(instruction next, chip8* system) {
  // Extract the value of NNN.
  uint16_t n = ((next.hi & 0x0F) << 8) + next.lo;
  // Store the current program counter on the stack.
  system->stack[system->sp] = system->pc;
  system->sp += 1;

  // Jump to the location of |n|.
  system->pc = n;
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
    uint16_t screen_index = (x + (7 - i)) + (32 * y);

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

  fprintf(stderr, "VX: %d VX %d n %d\n", system->V[x], system->V[y], n);

  // Set the initial state of VF to 0.
  system->V[0x0F] = 0;

  for (int i = 0; i < n; ++i) {
    // Load the row as bit-encoded memory.
    uint8_t row = system->memory[system->I + i];
    fprintf(stderr, "row: %d\n", row);
    draw_row(system->V[x], system->V[y] + i, row, system);
  }
}

void emulate_cycle(chip8* system) {
  // Determine next instrution.
  instruction next = get_instruction(system);

  switch (next.opcode) {
    case CLEAR_SCREEN:
      break;
    case RETURN:
      break;
    case JUMP:
      break;
    case CALL:
      break;
    case IF_X_EQ_NN:
      break;
    case IF_X_NEQ_NN:
      break;
    case IF_X_EQ_Y:
      break;
    case SET_X_NN:
      break;
    case ADD_X_NN:
      break;
    case SET_X_Y:
      break;
    case OR_X_Y:
      break;
    case AND_X_Y:
      break;
    case XOR_X_Y:
      break;
    case ADD_X_Y:
      break;
    case SUB_X_Y:
      break;
    default:
      break;
  }

  // act.
}