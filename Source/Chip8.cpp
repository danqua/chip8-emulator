#include "Chip8.h"

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
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
	ProgramCounter += 2;

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
			break;
		}
		break;
	case 0x1000:	// Jump
		ProgramCounter = Opcode & 0x0FFF;
		break;
	case 0x2000:	// Call
		StackPointer++;
		Stack[StackPointer] = ProgramCounter;
		ProgramCounter = Opcode & 0x0FFF;
		break;
	case 0x3000: // Skip next instruction (SE Vx, byte)
	{
		if (Registers[Vx] == (Opcode & 0x00FF))
		{
			ProgramCounter += 2;
		}
	}
	break;
	case 0x4000: // Skip next instruction not equal (SNE Vx, byte)
		if (Registers[Vx] != (Opcode & 0x00FF))
		{
			ProgramCounter += 2;
		}
		break;
	case 0x5000: // Skip next instruction equal (SE Vx, Vy)
		if (Registers[Vx] == Registers[Vy])
		{
			ProgramCounter += 2;
		}
		break;
	case 0x6000:	// Sets register
		Registers[Vx] = Opcode & 0x00FF;
		break;
	case 0x7000:	// Adds value to register
		Registers[Vx] += Opcode & 0x00FF;
		ProgramCounter += 2;
		break;
	case 0x8000:
		switch (Opcode & 0x000F)
		{
		case 0x0000:	// Set Vx = Vy
			Registers[Vx] = Registers[Vy];
			break;
		case 0x0001:	// Set Vx = Vx OR Vy
			Registers[Vx] = Registers[Vx] | Registers[Vy];
			break;
		case 0x0002:	// Set Vx = Vx AND Vy
			Registers[Vx] = Registers[Vx] & Registers[Vy];
			break;
		case 0x0003:	// Set Vx = Vx XOR Vy
			Registers[Vx] = Registers[Vx] ^ Registers[Vy];
			break;
		case 0x0004:	// Set Vx = Vx + Vy, set VF = carry.
		{
			if (Registers[Vx] + Registers[Vy] > 255)
			{
				Registers[0xF] = 1;
			}
			else
			{
				Registers[0xF] = 0;
			}

			Registers[Vx] = Registers[Vx] + Registers[Vy];
			break;
		}
		case 0x0005:	// Set Vx = Vx - Vy, set VF = NOT borrow.
		{
			if (Registers[Vx] > Registers[Vy])
			{
				Registers[0xF] = 1;
			}
			else
			{
				Registers[0xF] = 0;
			}

			Registers[Vx] = Registers[Vx] - Registers[Vy];
			break;
		}
		case 0x0006:	// Set Vx = Vx SHR 1.
		{
			if (Registers[Vx] & 0x01)
			{
				Registers[0xF] = 1;
			}
			else
			{
				Registers[0xF] = 0;
			}

			Registers[Vx] = Registers[Vx] >> 2;
			break;
		}
		case 0x0007:	// Set Vx = Vy - Vx, set VF = NOT borrow.
		{
			if (Registers[Vy] > Registers[Vx])
			{
				Registers[0xF] = 1;
			}
			else
			{
				Registers[0xF] = 0;
			}

			Registers[Vx] = Registers[Vy] - Registers[Vx];
			break;
		}
		case 0x000E:
		{
			if (Registers[Vx] & 0x80)
			{
				Registers[0xF] = 1;
			}
			else
			{
				Registers[0xF] = 0;
			}
			Registers[Vx] = Registers[Vx] << 2;
			break;
		}
		}
	break;
	case 0x9000:	// Skip next instruction if Vx != Vy (SNE Vx, Vy)
	{
		if (Registers[Vx] != Registers[Vy])
		{
			ProgramCounter += 2;
		}
		break;
	}
	case 0xA000:	// Sets the index register
		IndexRegister = Opcode & 0x0FFF;
		break;
	case 0xB000:	// Jump to location nnn + V0
		ProgramCounter = (Opcode & 0x0FFF) + Registers[0];
		break;
	case 0xC000:	// Set Vx = random byte AND kk
	{
		byte RandomNumber = rand() % 0xFF;
		Registers[Vx] = RandomNumber & (Opcode & 0x00FF);
	}
	break;
	case 0xD000:	// Display n-byte sprite
	{
		byte NumRows = Opcode & 0x000F;

		byte XCoordinate = Registers[Vx];
		byte YCoordinate = Registers[Vy];


		// Wrap sprite if starting position is outside screen
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
	}
	break;
	case 0xE000:
	{
		switch (Opcode & 0x00FF)
		{
		case 0x009E:	// Skip next instruction if key with the value of Vx is pressed.
		{
			if (Keypad == Registers[Vx])
			{
				ProgramCounter += 2;
			}
			break;
		}
		case 0x00A1:	// Skip next instruction if key with the value of Vx is not pressed.
		{
			if (Keypad != Registers[Vx])
			{
				ProgramCounter += 2;
			}
			break;
		}
		}
		break;
	}
	case 0xF000:
		switch (Opcode & 0x00FF)
		{
			case 0x0007:	// Set Vx = delay timer value.
				Registers[Vx] = DelayTimer;
			break;
			case 0x000A:	// Wait for a key press, store the value of the key in Vx.
				// TODO
			break;
			case 0x0015: // Set delay timer = Vx.
				DelayTimer = Registers[Vx];
			break;
			case 0x0018:	// Set sound timer = Vx.
				SoundTimer = Registers[Vx];
			break;
			case 0x001E:	// Set I = I + Vx.
				IndexRegister += Registers[Vx];
			break;
			case 0x0029:	// Set I = location of sprite for digit Vx.
				// TODO
				IndexRegister = Registers[Vx];
			break;
			case 0x0033:	// Store BCD representation of Vx in memory locations I, I+1, and I+2.
				Memory[IndexRegister] = Registers[Vx] / 100;
				Memory[IndexRegister + 1] = (Registers[Vx] / 10) % 10;
				Memory[IndexRegister + 2] = Registers[Vx] % 10;
			break;
			case 0x0055:	// Store registers V0 through Vx in memory starting at location I.
			{
				for (byte i = 0; i < Registers[Vx]; i++)
				{
					Memory[IndexRegister + i] = Registers[i];
				}
				break;
			}
			case 0x0065:	// Read registers V0 through Vx from memory starting at location I.
			{
				for (byte i = 0; i < Memory[IndexRegister]; i++)
				{
					Registers[i] = Memory[IndexRegister + i];
				}
				break;
			}
		}
	break;
	default:
		printf("Unknown opcode: 0x%X\n", Opcode);
	}
}