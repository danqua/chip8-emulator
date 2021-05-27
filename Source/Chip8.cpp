#include "Chip8.h"

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>

bool cpu::LoadROM(const char* Filename)
{
	FILE* FilePtr = fopen(Filename, "rb");

	if (!FilePtr)
	{
		printf("Failed to load ROM '%s'\n", Filename);
		return false;
	}

	// Read file
	fseek(FilePtr, 0, SEEK_END);
	long FileSizeInBytes = ftell(FilePtr);
	rewind(FilePtr);
	fread(&Memory[0x200], 1, FileSizeInBytes, FilePtr);
	fclose(FilePtr);

	return true;
}

void cpu::Initialize()
{
	// Reset memory
	memset(Memory, 0, sizeof(byte) * 4096);

	// Load font into memory
	memcpy(Memory, Font, sizeof(Font));

	// Reset registers
	memset(Registers, 0, sizeof(byte) * 16);

	// Reset stack
	memset(Stack, 0, sizeof(word) * 16);

	// Set the PC to start of most chip-8 programs
	ProgramCounter = 0x200;

	// Set the index register to 0
	IndexRegister = 0;

	// Clear the screen
	ClearDisplay();
}

void cpu::ClearDisplay()
{
	memset(Display, 0, sizeof(byte) * 64 * 32);
}

void cpu::Fetch()
{
	// 0xA2 << 8 => 0xA200 | 0x12 => 0xA212
	Opcode = Memory[ProgramCounter] << 8 | Memory[ProgramCounter + 1];
}

void cpu::Execute()
{

	byte Vx = (Opcode & 0x0F00) >> 8;
	byte Vy = (Opcode & 0x00F0) >> 4;

	// Decode opcode by looking at the first nibble
	switch (Opcode & 0xF000)
	{
	case 0x0000:
		switch (Opcode & 0x00FF)
		{
		case 0x00E0:	// Clear the display
			ClearDisplay();
			ProgramCounter += 2;
			break;
		}
		break;
	case 0x1000:	// Jump
		ProgramCounter = Opcode & 0x0FFF;
		break;
	case 0x6000:	// Sets register
		Registers[Vx] = Opcode & 0x00FF;
		ProgramCounter += 2;
		break;
	case 0x7000:	// Adds value to register
		Registers[Vx] += Opcode & 0x00FF;
		ProgramCounter += 2;
		break;
	case 0xA000:	// Sets the index register
		IndexRegister = Opcode & 0x0FFF;
		ProgramCounter += 2;
		break;
	case 0xD000:	// Display n-byte sprite
	{
		byte NumRows = Opcode & 0x000F;

		byte XCoordinate = Registers[Vx];
		byte YCoordinate = Registers[Vy];


		// Wrap sprite if starting position is outside screen dimension
		XCoordinate = XCoordinate % 64;
		YCoordinate = YCoordinate % 32;

		// Set the carry flag to zero
		Registers[0xF] = 0;

		for (byte y = 0; y < NumRows; y++)
		{
			byte SpriteData = Memory[IndexRegister + y];

			for (byte x = 0; x < 8; x++)
			{
				if ((SpriteData & (0x80 >> x)) != 0)
				{
					if (Display[(XCoordinate + x + (YCoordinate + y) * 64)] == 1)
					{
						Registers[0xF] = 1;
					}
					Display[(XCoordinate + x + (YCoordinate + y) * 64)] ^= 1;
				}
			}
		}

		DrawFlag = true;
		ProgramCounter += 2;
	}
	break;
	default:
		printf("Unknown opcode: 0x%X\n", Opcode);
	}
}