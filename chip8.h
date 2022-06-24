#include <stdint.h>

#include <SDL2/SDL.h>

#define FONT_SIZE 80
#define CLOCK_SPEED 1000000

typedef enum opcode {
  UNKNOWN = 0,
  // 0NNN,
  CLEAR_SCREEN,   // 0x00E0
  RETURN,         // 0x00EE
  JUMP,           // 0x1NNN
  CALL,           // 0x2NNN
  IF_X_EQ_NN,     // 0x3XNN
  IF_X_NEQ_NN,    // 0x4XNN
  IF_X_EQ_Y,      // 0x5XY0
  SET_X_NN,       // 0x6XNN
  ADD_X_NN,       // 0x7xNN
  SET_X_Y,        // 0x8XY0
  OR_X_Y,         // 0x8XY1
  AND_X_Y,        // 0x8XY2
  XOR_X_Y,        // 0x8XY3
  ADD_X_Y,        // 0x8XY4
  SUB_X_Y,        // 0x8XY5
  SHIFT_X_RIGHT,  // 0x8XY6
  SUB_X_Y_REV,    // 0x8XY7
  SHIFT_X_LEFT,   // 0x8XYE
  IF_X_NEQ_Y,     // 0x9XY0
  SET_I_NNN,      // 0xANNN
  JUMP_ADDR,      // 0xBNNN
  SET_RAND,       // 0xCXNN
  DRAW,           // 0xDXYN
  IF_KEY_EQ,      // 0xEX9E
  IF_KEY_NEQ,     // 0xEXA1
  GET_DELAY,      // 0xFX07
  GET_KEY,        // 0xFX0A
  SET_DELAY,      // 0xFX15
  SET_SOUND,      // 0xFX18
  ADD_X_I,        // 0xFX1E
  LOAD_CHAR,      // 0xFX29
  BCD,            // 0xFX33
  REG_DUMP,       // 0xFX55
  REG_LOAD,       // 0xFX65
} opcode_t;

typedef struct instruction {
  opcode_t opcode;
  uint8_t hi;
  uint8_t lo;
} instruction;

void print_instruction(instruction i);

typedef struct chip8 {
  // The Chip 8 has 4k of memory in total.
  uint8_t memory[4096];

  // Registers.
  // The Chip 8 system has 15 general purpose registers numbered V0 - VE.
  // The 16th register (VF) is used as a 'carry flag' for some instructions.
  uint8_t V[16];

  // Index register I used as a memory address.
  // Has a maximum value of 0xFFF.
  uint16_t I;

  // Program Counter, has a maximum value of 0xFFF.
  uint16_t pc;

  // The stack used to store the value of |pc| before calling a subroutine.
  uint16_t stack[16];

  // Represents the current position on the stack.
  uint8_t sp;

  // Stores the state of the 16 keys.
  uint8_t keys[16];

  // The Chip 8 has a monochrome screen with a 64 x 32 resolution.
  uint8_t screen[64 * 32];

  // Whether to skip the following instruction.
  uint8_t skip;

  // Used for timing events. Counts down at 60Hz.
  uint8_t delay_timer;

  // Used for sound effects. A beeping sound is played whenever |sound_timer| is
  // nonzero. Counts down at 60Hz.
  uint8_t sound_timer;

  // Indicates that a "DRAW" operation has been performed on the previous cycle
  // and that the screen should be updated.
  uint8_t draw_flag;

  // Flag used to indicate that a "jump-like" instruction has just been
  // executed. If this flag is set then the Program Counter isn't incremented
  // for the cycle.
  uint8_t jumped;

  uint64_t cycle;
} chip8;

// Extracts the value of N from the instruction |i| in the form:
//
//   ___N
uint8_t n(instruction i);

// Extracts the value of NNN from the instruction |i| in the form:
//
//  _NNN
uint16_t nnn(instruction i);

// Extracts the value of X from the instruction |i| in the form:
//
//  _X__
uint8_t x(instruction i);

// Extracts the value of Y from the given instruction |i|.
//
//  __Y_
uint8_t y(instruction i);

// Load the ascii font sprites into |system|'s memory.
void load_hex_fonts(chip8* system);

// Load the contents of |filename| into |system|. If the size of the file at
// |filename| exceeds the available memory in |system| then a nonzero value will
// be returned.
int load_program(const char* filename, chip8* system);

// Create a new chip8 struct with it's values initialized.
chip8 initialize_chip8();

// Print the contents of |system| for debugging purposes.
void print_chip8(chip8 system);

instruction get_instruction(const chip8* system);

void set_register(uint8_t vx, uint8_t val, chip8* system);

// Performs the CLEAR_SCREEN instruction.
//
// NOTE: The values in |next| are not required to perform this instruction, but
// it is passed anyways for debugging purposes.
void clear_screen(instruction next, chip8* system);

// Performs the RETURN operation.
// Pops the old value of |pc| off the stack, and returns to that position of the
// program.
//
// NOTE: The values in |next| are not required to perform this instruction, but
// it is passed anyways for debugging purposes.
void return_subroutine(instruction next, chip8* system);

// Performs the JUMP operation.
// For the given instruction 0x1NNN jumps to address NNN.
void jump(instruction next, chip8* system);

// Performs the CALL operation.
// For the given instruction 0x2NNN calls the subroutine at address NNN.
void call(instruction next, chip8* system);

// Performs the IF_X_EQ_NN instruction.
// For the given instruction 0x3XNN skips the next instruction if VX == NN.
void if_x_eq_nn(instruction next, chip8* system);

// Performs the IF_X_NEQ_NN instruction.
// For the given instruction 0x3XNN skips the next instruction if VX != NN.
void if_x_neq_nn(instruction next, chip8* system);

// Performs the IF_X_EQ_Y instruction.
// For the given instruction 0x3XNN skips the next instruction if VX == VY.
void if_x_eq_y(instruction next, chip8* system);

// Performs the SET_X_NN instruction.
// For the given instruction 0x6XNN sets register VX to the constant NN.
void set_x_nn(instruction next, chip8* system);

// Performs the ADD_X_NN instruction.
// For the given instruction 0x7XNN sets VX = NN.
// NOTE: The carry flag is not changed by this operation.
void add_x_nn(instruction next, chip8* system);

// Performs the SET_X_Y instruction.
// For the given instruction 0x8XY0 sets VX = VY.
void set_x_y(instruction next, chip8* system);

// Performs the OR_X_Y instruction.
// For the given instruction 0x8XY1 sets VX = VX | VY.
void or_x_y(instruction next, chip8* system);

// Performs the ADD_X_Y instruction.
// For the given instruction 0x8XY2 sets VX = VX & VY.
void and_x_y(instruction next, chip8* system);

// Performs the XOR_X_Y instruction.
// For the given instruction 0x8XY3 sets VX = VX ^ VY.
void xor_x_y(instruction next, chip8* system);

// Performs the ADD_X_Y instruction.
// For the given instruction 0x8XY4 sets VX = VX + VY.
// VF is set to 1 when there's a carry, and to 0 when there is not.
void add_x_y(instruction next, chip8* system);

// Performs the SUB_X_Y instruction.
// For the given instruction 0x8XY5 sets VX = VX - VY.
// VF is set to 0 when there's a borrow, and 1 when there is not.
void sub_x_y(instruction next, chip8* system);

// Performs the SHIFT_X_RIGHT instruction.
// For the given instruction 0x8XY6:
//   - stores least significant bit of VX in VF
//   - shifts VX to the right by 1
void shift_x_right(instruction next, chip8* system);

// Performs the SUB_X_Y_REV instruction.
// For the given instruction 0x8XY7 sets VX = VY - VX.
// VF is set to 0 when there's a borrow, and 1 when there is not.
void sub_x_y_rev(instruction next, chip8* system);

// Performs the SHIFT_X_LEFT instruction.
// For the given instruction 0x8XYE:
//   - stores most significant bit of VX in VF
//   - shifts VX to the left by 1
void shift_x_left(instruction next, chip8* system);

// Performs the IF_X_NEQ_Y instruction.
// For the given instruction 0x9XY0 skips the next instruction if VX == VY.
void if_x_neq_y(instruction next, chip8* system);

// Performs the SET_I_NNN instruction.
// For the given instruction 0xANNN sets I = NNN.
void set_i_nnn(instruction next, chip8* system);

// Perform the JUMP_ADDR instruction.
// For the given instruction 0xBNNN jumps to the address of V0 + NNN.
void jump_addr(instruction next, chip8* system);

// Performs the SET_RAND instruction.
// For the given instruction 0xCXNN sets VX = rand() & NN.
//
// NOTE: Assumes that stdlib rand() has already been seeded.
void set_rand(instruction next, chip8* system);

// Performs the DRAW instruction.
// For the given instruction 0xDXYN draws a sprite at coordinate (VX, VY) to the
// screen.
//
// The sprite is 8 pixels wide and N pixels in height.
//
// The sprite is loaded from memory as bit-encoded rows starting from I.
// I is unchanged by the operation.
//
// VF is set to 1 if any screen pixels are flipped from set to unset when the
// sprite is drawn. Otherwise, it is set to 0.
void draw(instruction next, chip8* system);

// Performs the IF_KEY_EQ instruction.
// For the given instruction 0xEX9E skips the next instruction if the key
// corresponding to the value of VX is pressed.
void if_key_eq(instruction next, chip8* system);

// Performs the IF_KEY_NEQ instruction.
// For the given instruction 0xEXA1 skips the next instruction if the key
// corresponding to the value of VX is NOT pressed.
void if_key_neq(instruction next, chip8* system);

// Performs the GET_DELAY instruction.
// For the given instruction 0xFX07 sets VX = delay_timer.
void get_delay(instruction next, chip8* system);

// Performs the GET_KEY instruction.
// For the given instruction 0xFX0A waits for a key press and stores it's value
// in VX.
//
// NOTE: This is a blocking operation. All processing is halted until the next
//       key event.
void get_key(instruction next, chip8* system);

// Performs the SET_DELAY instruction.
// For the given instruction 0xFX15 sets delay_timer = VX
void set_delay(instruction next, chip8* system);

// Performs the SET_SOUND instruction.
// For the given instruction 0xFX18 sets sound_timer = VX
void set_sound(instruction next, chip8* system);

// Performs the ADD_X_I instruction.
// For the given instruction 0xFX1E adds VX to I.
// NOTE: The carry flag (VF) is not affected.
void add_x_i(instruction next, chip8* system);

// Performs the LOAD_CHAR instruction.
// For the given instruction 0xFX29 sets I to the location of the sprite for the
// character in VX.
void load_char(instruction next, chip8* system);

// Performs the BCD instruction.
// For the given instruction 0xFX33:
//   - Extract the value of VX
//   - Store the value of the hundreds digit at I
//   - Store the value of the tens     digit at I+1
//   - Store the value of the ones     digit at I+2
void bcd(instruction next, chip8* system);

// Performs the REG_DUMP instruction.
// For the given instruction 0xFX55
//   - stores V0 to VX in memory starting from I
//   - I is not modified by the operation.
void reg_dump(instruction next, chip8* system);

// Performs the REG_LOAD instruction.
// For the given instruction 0xFX65
//   - loads V0 to VX from memory starting at I
//   - I is not modified by the operation.
void reg_load(instruction next, chip8* system);

void emulate_cycle(chip8* system);

// Returns a nonzero value if |keycode| represents a key on the chip8 hex
// keypad.
uint8_t is_chip8_key(SDL_Keycode keycode);

// Returns the hex-key value which corresponds to |keycode|.
uint8_t hex_keycode(SDL_Keycode keycode);