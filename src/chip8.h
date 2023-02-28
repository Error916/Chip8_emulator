#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#define START_ADDRESS 0x0200

#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

#define KEY_COUNT 16
#define MEMORY_SIZE 0x1000
#define VIDEO_SIZE 0x0800
#define REGISTER_COUNT 16
#define STACK_LEVELS 16
#define VIDEO_HEIGHT 32
#define VIDEO_WIDTH 64

typedef void (*Chip8Func)(void *);

typedef struct {
	uint8_t registers[REGISTER_COUNT];
	uint8_t memory[MEMORY_SIZE];
	uint16_t index;
	uint16_t pc;
	uint16_t stack[STACK_LEVELS];
	uint8_t sp;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint8_t keypad[KEY_COUNT];
	uint32_t video[0x0800];
	uint16_t opcode;

	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];
} CPU;

static const uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void createCPU(CPU *cpu);
void destroyCPU(CPU *cpu);

void LoadROM(CPU *cpu, const char *filename);
uint8_t RandByte(void);

void Chip8Cycle(CPU *cpu);

void Table0(CPU *cpu);
void Table8(CPU *cpu);
void TableE(CPU *cpu);
void TableF(CPU *cpu);
void OP_NULL(CPU *cpu);

void OP_1nnn(CPU *cpu);	// JMP
void OP_2nnn(CPU *cpu);	// CALL
void OP_3xkk(CPU *cpu);	// SE Vx, byte
void OP_4xkk(CPU *cpu);	// SNE Vx, byte
void OP_5xy0(CPU *cpu); // SE Vx, Vy
void OP_6xkk(CPU *cpu);	// LD Vx, byte
void OP_7xkk(CPU *cpu);	// ADD Vx, byte
void OP_9xy0(CPU *cpu);	// SNE Vx, Vy
void OP_Annn(CPU *cpu);	// LD I, addr
void OP_Bnnn(CPU *cpu);	// JP V0, addr
void OP_Cxkk(CPU *cpu);	// RND Vx, byte
void OP_Dxyn(CPU *cpu);	// DRW Vx, Vy, nibble

void OP_8xy0(CPU *cpu);	// LD Vx, Vy
void OP_8xy1(CPU *cpu);	// OR Vx, Vy
void OP_8xy2(CPU *cpu);	// AND Vx, Vy
void OP_8xy3(CPU *cpu);	// XOR Vx, Vy
void OP_8xy4(CPU *cpu);	// ADD Vx, Vy
void OP_8xy5(CPU *cpu);	// SUB Vx, Vy
void OP_8xy6(CPU *cpu);	// SHR Vx
void OP_8xy7(CPU *cpu);	// SUBN Vx, Vy
void OP_8xyE(CPU *cpu);	// SHL Vx {, Vy}

void OP_00E0(CPU *cpu);	// CLS
void OP_00EE(CPU *cpu);	// RET

void OP_Ex9E(CPU *cpu);	// SKP Vx
void OP_ExA1(CPU *cpu);	// SKNP Vx
void OP_Fx07(CPU *cpu);	// LD Vx, DT
void OP_Fx0A(CPU *cpu);	// LD Vx, K
void OP_Fx15(CPU *cpu);	// LD DT, Vx
void OP_Fx18(CPU *cpu);	// LD ST, Vx
void OP_Fx1E(CPU *cpu);	// ADD I, Vx
void OP_Fx29(CPU *cpu);	// LD F, Vx
void OP_Fx33(CPU *cpu); // LD B, Vx
void OP_Fx55(CPU *cpu); // LD [I], Vx
void OP_Fx65(CPU *cpu); // LD Vx, [I]
