#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_audio.h>
#include <iostream>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device device;
bool beepactive = false;

Uint8* audiobuff;
Uint32 audiolen;
int AudioBuffByteOffset = 0;
float AudioVolume = 0.5;

Uint8* beepbuff;
Uint32 beeplen;
int beepBuffByteOffset = 0;
float beepVolume = 0.6;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window = nullptr;
SDL_Surface* screenSurface = nullptr;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	Uint8* pOutputF32 = audiobuff + AudioBuffByteOffset;

	float* outputCopy = new float[frameCount * 4];
	float* outputSamples = (float*)pOutputF32;
	for (int i = 0; i < frameCount * 2; i++)
	{
		outputCopy[i] = outputSamples[i] * AudioVolume;
	}

	if (beepactive)
	{
		if (beepBuffByteOffset + frameCount * sizeof(float) > beeplen)
		{
			beepactive = false;
			beepBuffByteOffset = 0;
		}
		else
		{
			float* BeepSamples = (float*)(beepbuff + beepBuffByteOffset);
			for (int i = 0; i < frameCount * 2; i++)
			{
				outputCopy[2 * i] += BeepSamples[i] * beepVolume;
				outputCopy[2 * i + 1] += BeepSamples[i] * beepVolume;
			}

			beepBuffByteOffset += sizeof(float) * frameCount;
		}
	}

	//looping
	if (AudioBuffByteOffset + frameCount * sizeof(float) * 2 > audiolen)
	{
		int RestOfSoundLength = audiolen - AudioBuffByteOffset;
		SDL_memcpy(pOutput, audiobuff + AudioBuffByteOffset, RestOfSoundLength);
		int BegginingLength = (frameCount * sizeof(float) * 2) - RestOfSoundLength;
		SDL_memcpy(pOutput, audiobuff, BegginingLength);
		AudioBuffByteOffset = BegginingLength;
		return;
	}
	
	
	SDL_memcpy(pOutput, outputCopy,sizeof(float) * frameCount * 2);
	AudioBuffByteOffset += sizeof(float) * frameCount * 2;

	delete[] outputCopy;
}

void Init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << "SDL could not be initialized\n";
		exit(-1);
	}

	window = SDL_CreateWindow("window", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	if (window == nullptr)
	{
		std::cout << "SDL window could not be created\n";
		exit(-1);
	}
	screenSurface = SDL_GetWindowSurface(window);

	SDL_AudioSpec AudioSpec;

	if (!SDL_LoadWAV("melody.wav", &AudioSpec, &audiobuff, &audiolen))
	{
		std::cout << SDL_GetError();
	}

	if (!SDL_LoadWAV("beep.wav", &AudioSpec, &beepbuff, &beeplen))
	{
		std::cout << SDL_GetError();
	}

	ma_device_config deviceconfig = ma_device_config_init(ma_device_type_playback); //automatically sets values to device's native configuration
	deviceconfig.dataCallback = data_callback;


	if (ma_device_init(NULL, &deviceconfig, &device) != MA_SUCCESS)
	{
		std::cout << "miniaudio melody device could not be initialized\n";
		exit(-1);
	}

	ma_device_info device_info;
	if (ma_device_get_info(&device, ma_device_type_playback, &device_info) != MA_SUCCESS)
	{
		SDL_Log("Failed to query info about Signals device!");
	}
}

void PlayBeepSound()
{
	if (!beepactive)
	{
		beepactive = true;
	}
}

void DecreaseMusicVolume()
{
	if (AudioVolume > 0)
	{
		AudioVolume -= 0.1;
	}
}

void IncreaseMusicVolume()
{
	if (AudioVolume < 1)
	{
		AudioVolume += 0.1;
	}
}

void DeInit()
{
	ma_device_uninit(&device);
}

int main(int argc, char* args[])
{
	Init();
	ma_device_start(&device);

	SDL_Event event;
	bool quit = false;

	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				quit = true;
				break;
			}

			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				if (event.key.key == SDLK_SPACE)
				{
					PlayBeepSound();
				}
				if (event.key.key == SDLK_UP)
				{
					IncreaseMusicVolume();
				}
				if (event.key.key == SDLK_DOWN)
				{
					DecreaseMusicVolume();
				}
			}
		}
	}


	DeInit();
	return 0;
}

