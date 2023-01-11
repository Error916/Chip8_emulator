#include "chip8.h"

void createCPU(CPU *cpu) {
	memset(cpu->registers, 0, 16 * sizeof (uint8_t));
	memset(cpu->memory, 0, 0x1000 * sizeof(uint8_t));
	cpu->index = 0;
	cpu->pc = START_ADDRESS;
	memset(cpu->stack, 0 ,16 * sizeof(uint16_t));
	cpu->sp = 0;
	cpu->delayTimer = 0;
	cpu->soundTimer = 0;
	memset(cpu->keypad, 0, 16 * sizeof(uint8_t));
	memset(cpu->video, 0, 0x0800 * sizeof(uint32_t));
	cpu->opcode = 0;

	for(uint8_t i = 0; i < FONTSET_SIZE; ++i)
		cpu->memory[FONTSET_START_ADDRESS + i] = fontset[i];
}

void destroyCPU(CPU *cpu) {
	(void) cpu;
}

void LoadROM(CPU *cpu, const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL) assert(0 && "Could not open file");
	fseek(fp, 0, SEEK_END);
	const size_t len = ftell(fp);
	uint8_t buffer[len];
	fseek(fp, 0, 0);
	fread(buffer, 1, len, fp);
	fclose(fp);

	for (size_t i = 0; i < len; ++i)
		cpu->memory[START_ADDRESS + i] = buffer[i];
}

