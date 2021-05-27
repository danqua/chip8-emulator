#pragma once

using byte = unsigned char;
using word = unsigned short;

// Font sprites
static const byte Font[] = {
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


struct cpu
{
	byte Memory[4096];			// 4096 bytes of memory.
	byte Registers[16];			// 16 8-bit registers.
	word Stack[16];				// Stores the address that the interpreter should return to when finished with a subroutine.
	word IndexRegister;			// Special 16-bit register to store memory addresses.
	byte DelayTimerRegister;	// When non-zero, it automatically decrements at a rate of 60Hz.
	byte SoundTimerRegister;	// When non-zero, it automatically decrements at a rate of 60Hz.
	word ProgramCounter;		// Stores the currently executing address.
	byte StackPointer;			// Points to the topmost level of the stack.
	byte Keypad;				// 16-key hexadecimal keypad
	byte Display[64 * 32];			// 64 * 32 pixel monochrome display
	word Opcode;				// Caches the current opcode
	bool DrawFlag;				// Indicates if the screen should be redrawn

	// Loads ROM into memory
	bool LoadROM(const char* Filename);

	// Resets the cpu and all it's states
	void Initialize();

	// Clears the display buffer
	void ClearDisplay();

	// Fetch the instruction from memory at the current PC
	void Fetch();

	// Decodes the instruction and executes it
	void Execute();
};