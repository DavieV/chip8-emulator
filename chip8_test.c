#include "chip8.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_clear_screen() {
  // Setup |next| to represent the CLEAR_SCREEN operation (0x00E0).
  instruction next;
  next.hi = 0x00;
  next.lo = 0xE0;

  chip8 system = initialize_chip8();
  // Set each value in the screen.
  memset(system.screen, 1, sizeof(system.screen));

  clear_screen(next, &system);

  uint8_t not_cleared = 0;
  for (int i = 0; i < sizeof(system.screen); ++i) {
    if (system.screen[i] != 0) {
      not_cleared = 1;
    }
  }
  assert(not_cleared == 0);
}

void test_return_subroutine() {
  // Setup |next| to represent the RETURN operation (0x00EE).
  instruction next;
  next.hi = 0x00;
  next.lo = 0xEE;

  chip8 system = initialize_chip8();
  // Place some state onto the stack.
  system.stack[0] = 0;
  system.stack[1] = 1;
  system.stack[2] = 2;
  system.sp = 3;

  return_subroutine(next, &system);
  assert(system.pc == 2);
  assert(system.sp == 2);

  return_subroutine(next, &system);
  assert(system.pc == 1);
  assert(system.sp == 1);

  return_subroutine(next, &system);
  assert(system.pc == 0);
  assert(system.sp == 0);
}

void test_jump() {
  chip8 system = initialize_chip8();
  // Setup |next| to represent the JUMP operation (0x1NNN).
  instruction next;
  next.hi = 0x1E;
  next.lo = 0x1A;
  jump(next, &system);
  assert(system.pc == 0xE1A);
}

void test_call() {
  // Setup |next| to represent the CALL operation (0x2NNN).
  instruction next;
  next.hi = 0x1E;
  next.lo = 0x1A;

  chip8 system = initialize_chip8();
  system.pc = 1234;

  call(next, &system);
  assert(system.stack[0] == 1234);
  assert(system.sp == 1);
  assert(system.pc == 0xE1A);
}

void test_if_x_eq_nn() {
  // Setup |next| to represent the IF_X_EQ_NN operation (0x3XNN).
  instruction next;
  next.hi = 0x30;
  next.lo = 15;

  chip8 system = initialize_chip8();
  system.V[0] = 14;
  if_x_eq_nn(next, &system);
  assert(system.skip == 0);

  system.V[0] = 15;
  if_x_eq_nn(next, &system);
  assert(system.skip == 1);
}

void test_if_x_neq_nn() {
  // Setup |next| to represent the IF_X_NEQ_NN operation (0x4XNN).
  instruction next;
  next.hi = 0x40;
  next.lo = 15;

  chip8 system = initialize_chip8();
  system.V[0] = 14;
  if_x_neq_nn(next, &system);
  assert(system.skip == 1);

  system.V[0] = 15;
  if_x_neq_nn(next, &system);
  assert(system.skip == 0);
}

void test_if_x_eq_y() {
  // Setup |next| to represent the IF_X_EQ_Y operation (0x5XY0).
  instruction next;
  next.hi = 0x50;
  next.lo = 0x10;

  chip8 system = initialize_chip8();
  system.V[0] = 14;
  system.V[1] = 15;
  if_x_eq_y(next, &system);
  assert(system.skip == 0);

  system.V[0] = 15;
  if_x_eq_y(next, &system);
  assert(system.skip == 1);
}

void test_set_x_nn() {
  chip8 system = initialize_chip8();
  instruction next;
  next.hi = 0x60;
  next.lo = 155;
  set_x_nn(next, &system);
  assert(system.V[0] == 155);

  next.hi = 0x68;
  next.lo = 38;
  set_x_nn(next, &system);
  assert(system.V[8] == 38);
}

void test_add_x_nn() {
  chip8 system = initialize_chip8();
  instruction next;
  next.hi = 0x71;
  next.lo = 10;
  add_x_nn(next, &system);
  assert(system.V[1] == 10);

  next.hi = 0x71;
  next.lo = 21;
  add_x_nn(next, &system);
  assert(system.V[1] == 31);
}

void test_set_x_y() {
  chip8 system = initialize_chip8();
  system.V[4] = 12;
  system.V[9] = 27;

  instruction next;
  next.hi = 0x84;
  next.lo = 0x90;
  set_x_y(next, &system);

  assert(system.V[4] == 27);
}

void test_or_x_y() {
  chip8 system = initialize_chip8();
  system.V[0] = 0b01010101;
  system.V[1] = 0b10101011;

  instruction next;
  next.hi = 0x80;
  next.lo = 0x11;
  or_x_y(next, &system);

  assert(system.V[0] == 0xFF);
}

void test_and_x_y() {
  chip8 system = initialize_chip8();
  system.V[0] = 0b01010101;
  system.V[1] = 0b10101011;

  instruction next;
  next.hi = 0x80;
  next.lo = 0x12;
  and_x_y(next, &system);

  assert(system.V[0] == 0b00000001);
}

void test_xor_x_y() {
  chip8 system = initialize_chip8();
  system.V[0] = 0b01010101;
  system.V[1] = 0b10101011;

  instruction next;
  next.hi = 0x80;
  next.lo = 0x13;
  xor_x_y(next, &system);

  assert(system.V[0] == 0b11111110);
}

void test_add_x_y() {
  // Setup |next| to represent the ADD_X_Y operation (0x8XY4).
  instruction next;
  next.hi = 0x80;
  next.lo = 0x14;

  chip8 system = initialize_chip8();

  // Test a case where the carry flag should not be set.
  system.V[0] = 6;
  system.V[1] = 5;
  add_x_y(next, &system);
  assert(system.V[0] == 11);
  assert(system.V[0xF] == 0);

  // Test a case where the carry flag should be set.
  system.V[0] = 128;
  system.V[1] = 129;
  add_x_y(next, &system);
  // Since the result of the addition produces an overflow, we expect V0 == 1.
  assert(system.V[0] == 1);
  assert(system.V[0xF] == 1);
}

void test_sub_x_y() {
  // Setup |next| to represent the SUB_X_Y operation (0x8XY5).
  instruction next;
  next.hi = 0x80;
  next.lo = 0x15;

  chip8 system = initialize_chip8();

  // Test a case where there is no borrow.
  system.V[0] = 6;
  system.V[1] = 5;
  sub_x_y(next, &system);
  assert(system.V[0] == 1);
  assert(system.V[0xF] == 1);

  // Test a case where there is a borrow.
  system.V[0] = 5;
  system.V[1] = 6;
  sub_x_y(next, &system);
  // Since the result of the subtraction produces an underflow,
  // we expect V0 == 255.
  assert(system.V[0] == 255);
  assert(system.V[0xF] == 0);
}

void test_shift_x_right() {
  // Setup |next| to represent the SHIFT_X_RIGHT operation (0x8XY6).
  instruction next;
  next.hi = 0x80;
  next.lo = 0x06;

  chip8 system = initialize_chip8();
  system.V[0] = 0xFF;
  shift_x_right(next, &system);
  assert(system.V[0xF] = 1);
  assert(system.V[0] == 0b01111111);
}

void test_shift_x_left() {
  // Setup |next| to represent the SHIFT_X_RIGHT operation (0x8XYE).
  instruction next;
  next.hi = 0x80;
  next.lo = 0x0E;

  chip8 system = initialize_chip8();
  system.V[0] = 0xFF;
  shift_x_left(next, &system);
  assert(system.V[0xF] = 1);
  assert(system.V[0] == 0b11111110);
}

void test_set_i_nnn() {
  chip8 system = initialize_chip8();
  // Setup |next| to represent the SET_I_NNN operation (0xANNN).
  instruction next;
  next.hi = 0xAE;
  next.lo = 0x1A;
  set_i_nnn(next, &system);
  assert(system.I == 0xE1A);
}

void test_jump_addr() {
  // Setup |next| to represent the JUMP_ADDR operation (0xBNNN).
  instruction next;
  next.hi = 0xBE;
  next.lo = 0x1A;

  chip8 system = initialize_chip8();
  jump_addr(next, &system);
  assert(system.pc == 0xE1A);
}

void test_set_rand() {
  // Setup |next| to represent the SET_RAND operation (0xCXNN).
  instruction next;
  next.hi = 0xC0;
  next.lo = 0xFF;

  chip8 system = initialize_chip8();

  // Seed the random number generator.
  // With the given seed, we expect 38 to be the first number generated.
  srand(0);

  set_rand(next, &system);
  assert(system.V[0] == 38);

  // The next number generated should be 39 (0b00100111)
  // We expect the result of (rand() & NN) to be 0b00000110.
  next.lo = 0b00011110;
  set_rand(next, &system);
  assert(system.V[0] == 0b00000110);
}

void test_draw() {
  // Setup |next| to represent the DRAW operation (0xDXYN).
  instruction next;
  next.hi = 0xD0;
  next.lo = 0x18;

  chip8 system = initialize_chip8();
  system.I = 0;
  system.V[0] = 0;
  system.V[1] = 0;

  // Store an 8x8 sprite in memory
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  //   1 1 1 1 1 1 1 1
  for (int i = 0; i < 8; ++i) {
    system.memory[i] = 0xFF;
  }

  draw(next, &system);

  // Verify the expected screen state.
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      assert(system.screen[i * 32 + j] == 1);
    }
  }
  assert(system.V[0x0F] == 0);

  next.lo = 0x11;
  system.V[0] = 16;
  system.V[1] = 20;
  system.screen[16 + 32 * 20] = 1;

  draw(next, &system);

  // Verify the expected screen state.
  assert(system.screen[16 + 32 * 20] == 0);
  assert(system.screen[17 + 32 * 20] == 1);
  assert(system.screen[18 + 32 * 20] == 1);
  assert(system.screen[19 + 32 * 20] == 1);
  assert(system.screen[20 + 32 * 20] == 1);
  assert(system.screen[21 + 32 * 20] == 1);
  assert(system.screen[22 + 32 * 20] == 1);
  assert(system.screen[23 + 32 * 20] == 1);

  assert(system.V[0x0F] == 1);
}

int main(int argc, char* argv[]) {
  fprintf(stderr, "Running tests...\n");

  // Call any test functions here.

  test_clear_screen();       // 0x00E0
  test_return_subroutine();  // 0x00EE
  test_jump();               // 0x1NNN
  test_call();               // 0x2NNN
  test_if_x_eq_nn();         // 0x3XNN
  test_if_x_neq_nn();        // 0x4XNN
  test_if_x_eq_y();          // 0x5XY0
  test_set_x_nn();           // 0x6XNN
  test_add_x_nn();           // 0x7XNN
  test_set_x_y();            // 0x8XY0
  test_or_x_y();             // 0x8XY1
  test_and_x_y();            // 0x8XY2
  test_xor_x_y();            // 0x8XY3
  test_add_x_y();            // 0x8XY4
  test_sub_x_y();            // 0x8XY5
  test_shift_x_right();      // 0x8XY6
  test_shift_x_left();       // 0x8XY7
  test_shift_x_left();       // 0x8XYE
  // 0x9XY0
  test_set_i_nnn();  // 0xANNN
  test_jump_addr();  // 0xBNNN
  test_set_rand();   // 0xCXNN
  test_draw();

  fprintf(stderr, "Tests completed successfully!\n");
  return 0;
}