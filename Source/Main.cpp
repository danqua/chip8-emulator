#include "Chip8.h"
#include <SDL.h>

int SDL_main(int argc, char** argv)
{
	SDL_Window* Window = SDL_CreateWindow(
		"Chip-8 Emulator",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		512,
		256,
		SDL_WINDOW_SHOWN
	);

	SDL_Surface* WindowSurface = SDL_GetWindowSurface(Window);

	SDL_Surface* Display = SDL_CreateRGBSurfaceWithFormat(0, 64, 32, 0, SDL_PIXELFORMAT_RGB888);

	Uint32* PixelBuffer = (Uint32*)Display->pixels;

	SDL_Event Event = {};

	bool IsRunning = true;

	Uint64 OldTime = SDL_GetPerformanceCounter();
	Uint64 ElapsedTime = 0;
	float DeltaTime = 0.0f;
	float FrequencyTimer = 0.0f;

	cpu CPU;

	CPU.Initialize();

	if (CPU.LoadROM("IBMLogo.ch8"))
	{
		while (IsRunning)
		{
			ElapsedTime = SDL_GetPerformanceCounter() - OldTime;
			OldTime = SDL_GetPerformanceCounter();
			DeltaTime = static_cast<float>(ElapsedTime) / SDL_GetPerformanceFrequency();
			
			FrequencyTimer += DeltaTime;

			if (FrequencyTimer > 1.0f / 60.0f)
			{
				CPU.Fetch();
				CPU.Execute(); 

				FrequencyTimer = 0.0f;
			}



			if (CPU.DrawFlag)
			{
				SDL_FillRect(Display, NULL, 0x000000);

				for (byte Y = 0; Y < 32; Y++)
				{
					for (byte X = 0; X < 64; X++)
					{
						PixelBuffer[X + Y * 64] = CPU.Display[X + Y * 64] == 0x01 ? 0xffffff : 0x000000;
					}
				}

				SDL_BlitScaled(Display, NULL, WindowSurface, NULL);
				SDL_UpdateWindowSurface(Window);
				CPU.DrawFlag = false;
			}

			



			
			while (SDL_PollEvent(&Event))
			{
				if (Event.type == SDL_QUIT)
				{
					IsRunning = false;
					break;
				}
			}
		}

	}

	return 0;
}