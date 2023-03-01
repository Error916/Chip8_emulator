#include "chip8.h"

static void zero(void *p, size_t len) {
    char *s = p;
    for (size_t i = 0; i < len; i++) {
        s[i] = 0;
    }
}

void createCPU(CPU *cpu) {
	zero(cpu, sizeof(*cpu));
	cpu->pc = START_ADDRESS;

	for(uint8_t i = 0; i < FONTSET_SIZE; ++i)
		cpu->memory[FONTSET_START_ADDRESS + i] = fontset[i];

	// Set up function pointer table
	cpu->table[0x0] = &Table0;
	cpu->table[0x1] = &OP_1nnn;
	cpu->table[0x2] = &OP_2nnn;
	cpu->table[0x3] = &OP_3xkk;
	cpu->table[0x4] = &OP_4xkk;
	cpu->table[0x5] = &OP_5xy0;
	cpu->table[0x6] = &OP_6xkk;
	cpu->table[0x7] = &OP_7xkk;
	cpu->table[0x8] = &Table8;
	cpu->table[0x9] = &OP_9xy0;
	cpu->table[0xA] = &OP_Annn;
	cpu->table[0xB] = &OP_Bnnn;
	cpu->table[0xC] = &OP_Cxkk;
	cpu->table[0xD] = &OP_Dxyn;
	cpu->table[0xE] = &TableE;
	cpu->table[0xF] = &TableF;

	for (size_t i = 0; i <= 0xE; i++) {
		cpu->table0[i] = &OP_NULL;
		cpu->table8[i] = &OP_NULL;
		cpu->tableE[i] = &OP_NULL;
	}

	cpu->table0[0x0] = &OP_00E0;
	cpu->table0[0xE] = &OP_00EE;

	cpu->table8[0x0] = &OP_8xy0;
	cpu->table8[0x1] = &OP_8xy1;
	cpu->table8[0x2] = &OP_8xy2;
	cpu->table8[0x3] = &OP_8xy3;
	cpu->table8[0x4] = &OP_8xy4;
	cpu->table8[0x5] = &OP_8xy5;
	cpu->table8[0x6] = &OP_8xy6;
	cpu->table8[0x7] = &OP_8xy7;
	cpu->table8[0xE] = &OP_8xyE;

	cpu->tableE[0x1] = &OP_ExA1;
	cpu->tableE[0xE] = &OP_Ex9E;

	for (size_t i = 0; i <= 0x65; i++)
		cpu->tableF[i] = &OP_NULL;

	cpu->tableF[0x07] = &OP_Fx07;
	cpu->tableF[0x0A] = &OP_Fx0A;
	cpu->tableF[0x15] = &OP_Fx15;
	cpu->tableF[0x18] = &OP_Fx18;
	cpu->tableF[0x1E] = &OP_Fx1E;
	cpu->tableF[0x29] = &OP_Fx29;
	cpu->tableF[0x33] = &OP_Fx33;
	cpu->tableF[0x55] = &OP_Fx55;
	cpu->tableF[0x65] = &OP_Fx65;
}

void destroyCPU(CPU *cpu) {
	(void) cpu;
}

uint8_t RandByte(uint64_t *s) {
	*s = *s*0x3243f6a8885a308d + 1;
	return *s >> 56;
}

void Chip8Cycle(CPU *cpu) {
	// Fetch
	cpu->opcode = (cpu->memory[cpu->pc] << 8u) | cpu->memory[cpu->pc + 1];

	// Increment the PC before we execute anything
	cpu->pc += 2;

	// Decode and Execute
	(*(cpu->table[(cpu->opcode & 0xF000u) >> 12u]))(cpu);

	// Decrement the delay timer if it's been set
	if (cpu->delayTimer > 0) cpu->delayTimer -= 1;

	// Decrement the sound timer if it's been set
	if (cpu->soundTimer > 0) cpu->soundTimer -= 1;
}

void Table0(CPU *cpu) {
	(*(cpu->table0[cpu->opcode & 0x000Fu]))(cpu);
}

void Table8(CPU *cpu) {
	(*(cpu->table8[cpu->opcode & 0x000Fu]))(cpu);
}

void TableE(CPU *cpu) {
	(*(cpu->tableE[cpu->opcode & 0x000Fu]))(cpu);
}

void TableF(CPU *cpu) {
	(*(cpu->tableF[cpu->opcode & 0x00FFu]))(cpu);
}

void OP_NULL(CPU *cpu) {
	(void) cpu;
}

void OP_1nnn(CPU *cpu) {	// JMP
	uint16_t addr = cpu->opcode & 0x0FFFu;
	cpu->pc = addr;
}

void OP_2nnn(CPU *cpu) {	// CALL
	uint16_t addr = cpu->opcode & 0x0FFFu;
	cpu->stack[cpu->sp] = cpu->pc;
	cpu->sp += 1;
	cpu->pc = addr;
}

void OP_3xkk(CPU *cpu) {	// SE Vx, byte
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	if (cpu->registers[Vx] == byte) cpu->pc += 2;

}

void OP_4xkk(CPU *cpu) {	// SNE Vx, byte
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	if (cpu->registers[Vx] != byte) cpu->pc += 2;
}

void OP_5xy0(CPU *cpu) {	// SE Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] == cpu->registers[Vy]) cpu->pc += 2;
}

void OP_6xkk(CPU *cpu) {	// LD Vx, byte
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	cpu->registers[Vx] = byte;
}

void OP_7xkk(CPU *cpu) {	// ADD Vx, byte
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	cpu->registers[Vx] += byte;
}

void OP_9xy0(CPU *cpu) {	// SNE Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] != cpu->registers[Vy]) cpu->pc += 2;
}

void OP_Annn(CPU *cpu) {	// LD I, addr
	uint16_t addr = cpu->opcode & 0x0FFFu;
	cpu->index = addr;
}

void OP_Bnnn(CPU *cpu) {	// JP V0, addr
	uint16_t addr = cpu->opcode & 0x0FFFu;
	cpu->pc = cpu->registers[0] + addr;
}

void OP_Cxkk(CPU *cpu) {	// RND Vx, byte
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	cpu->registers[Vx] = RandByte(&cpu->rng) & byte;
}

void OP_Dxyn(CPU *cpu) { // DRW Vx, Vy, nibble
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	uint8_t height = cpu->opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = cpu->registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = cpu->registers[Vy] % VIDEO_HEIGHT;

	cpu->registers[0xF] = 0;
	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = cpu->memory[cpu->index + row];
		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t *screenPixel = &cpu->video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) cpu->registers[0xF] = 1;
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void OP_8xy0(CPU *cpu) {	// LD Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] = cpu->registers[Vy];
}

void OP_8xy1(CPU *cpu) {	// OR Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] |= cpu->registers[Vy];
}

void OP_8xy2(CPU *cpu) {	// AND Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] &= cpu->registers[Vy];
}

void OP_8xy3(CPU *cpu) {	// XOR Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] ^= cpu->registers[Vy];
}

void OP_8xy4(CPU *cpu) {	// ADD Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	uint16_t sum = cpu->registers[Vx] + cpu->registers[Vy];
	if (sum > 255) cpu->registers[0xF] = 1;
	else cpu->registers[0xF] = 0;
	cpu->registers[Vx] = sum & 0xFFu;
}

void OP_8xy5(CPU *cpu) {	// SUB Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] > cpu->registers[Vy]) cpu->registers[0xF] = 1;
	else cpu->registers[0xF] = 0;
	cpu->registers[Vx] -= cpu->registers[Vy];
}

void OP_8xy6(CPU *cpu) {	// SHR Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[0xF] = (cpu->registers[Vx] & 0x1u);
	cpu->registers[Vx] >>= 1;
}

void OP_8xy7(CPU *cpu) {	// SUBN Vx, Vy
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vy] > cpu->registers[Vx]) cpu->registers[0xF] = 1;
	else cpu->registers[0xF] = 0;
	cpu->registers[Vx] = cpu->registers[Vy] - cpu->registers[Vx];
}

void OP_8xyE(CPU *cpu) {	// SHL Vx {, Vy}
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[0xF] = (cpu->registers[Vx] & 0x80u) >> 7u;
	cpu->registers[Vx] <<= 1;
}

void OP_00E0(CPU *cpu) {	// CLS
	zero(cpu->video, sizeof(cpu->video));
}

void OP_00EE(CPU *cpu) {	// RET
	cpu->sp -= 1;
	cpu->pc = cpu->stack[cpu->sp];
}

void OP_Ex9E(CPU *cpu) {	// SKP Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t key = cpu->registers[Vx];
	if (cpu->keypad[key]) cpu->pc += 2;
}

void OP_ExA1(CPU *cpu) {	// SKNP Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t key = cpu->registers[Vx];
	if (!cpu->keypad[key]) cpu->pc += 2;
}

void OP_Fx07(CPU *cpu) {	// LD Vx, DT
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[Vx] = cpu->delayTimer;
}

void OP_Fx0A(CPU *cpu) {	// LD Vx, K
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;

	if (cpu->keypad[0]) cpu->registers[Vx] = 0;
	else if (cpu->keypad[1]) cpu->registers[Vx] = 1;
	else if (cpu->keypad[2]) cpu->registers[Vx] = 2;
	else if (cpu->keypad[3]) cpu->registers[Vx] = 3;
	else if (cpu->keypad[4]) cpu->registers[Vx] = 4;
	else if (cpu->keypad[5]) cpu->registers[Vx] = 5;
	else if (cpu->keypad[6]) cpu->registers[Vx] = 6;
	else if (cpu->keypad[7]) cpu->registers[Vx] = 7;
	else if (cpu->keypad[8]) cpu->registers[Vx] = 8;
	else if (cpu->keypad[9]) cpu->registers[Vx] = 9;
	else if (cpu->keypad[10]) cpu->registers[Vx] = 10;
	else if (cpu->keypad[11]) cpu->registers[Vx] = 11;
	else if (cpu->keypad[12]) cpu->registers[Vx] = 12;
	else if (cpu->keypad[13]) cpu->registers[Vx] = 13;
	else if (cpu->keypad[14]) cpu->registers[Vx] = 14;
	else if (cpu->keypad[15]) cpu->registers[Vx] = 15;
	else cpu->pc -= 2;
}

void OP_Fx15(CPU *cpu) {	// LD DT, Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->delayTimer = cpu->registers[Vx];
}

void OP_Fx18(CPU *cpu) {	// LD ST, Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->soundTimer = cpu->registers[Vx];
}

void OP_Fx1E(CPU *cpu) {	// ADD I, Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->index += cpu->registers[Vx];
}

void OP_Fx29(CPU *cpu) {	// LD F, Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t digit = cpu->registers[Vx];

	cpu->index = FONTSET_START_ADDRESS + (5 * digit);
}

void OP_Fx33(CPU *cpu) { 	// LD B, Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t value = cpu->registers[Vx];

	// Ones-place
	cpu->memory[cpu->index + 2] = value % 10;
	value /= 10;

	// Tens-place
	cpu->memory[cpu->index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	cpu->memory[cpu->index] = value % 10;
}

void OP_Fx55(CPU *cpu) {	// LD [I], Vx
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
		cpu->memory[cpu->index + i] = cpu->registers[i];
}

void OP_Fx65(CPU *cpu) {	// LD Vx, [I]
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i)
		cpu->registers[i] = cpu->memory[cpu->index + i];
}
