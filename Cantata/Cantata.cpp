#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_audio.h>
#include <iostream>
#include <vector>
#include <immintrin.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device device;
bool beepactive = false;

Uint8* audiobuff;
Uint32 audiolen;
int AudioBuffByteOffset = 0;
float AudioVolume = 0.2;

Uint8* beepbuff;
Uint32 beeplen;
int beepBuffByteOffset = 0;
float beepVolume = 0.6;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window = nullptr;
SDL_Surface* screenSurface = nullptr;

ma_uint32 averageFrameCount = 480;
float* outputCopy = new float[averageFrameCount * 4];

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	Uint8* pOutputF32 = audiobuff + AudioBuffByteOffset;
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

			__m256 gainVector = _mm256_set1_ps(AudioVolume);
			__m256 beepGainVector = _mm256_set1_ps(beepVolume);

			for (int i = 0; i < frameCount * 2; i+=8)
			{
				if (i * sizeof(float) > audiolen) return;
				__m256 outputCopyValues = _mm256_load_ps(outputSamples + i);
				outputCopyValues = _mm256_mul_ps(gainVector, outputCopyValues);
				__m256 beepValues = _mm256_set_ps(BeepSamples[i / 2 + 3], BeepSamples[i / 2 + 3], BeepSamples[i / 2 + 2], BeepSamples[i / 2 + 2], BeepSamples[i / 2 + 1], BeepSamples[i / 2 + 1], BeepSamples[i / 2], BeepSamples[i / 2]);
				beepValues = _mm256_mul_ps(beepGainVector, beepValues);

				__m256 result = _mm256_add_ps(outputCopyValues, beepValues);

				float* addedFloats = (float*)&result;

				for (int j = 0; j < 8; j++)
				{
					outputCopy[i + j] = addedFloats[j];
				}
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
	AudioVolume = ma_clamp(AudioVolume - 0.1, 0, 1);
}

void IncreaseMusicVolume()
{
	AudioVolume = ma_clamp(AudioVolume + 0.1, 0, 1);
}

void SkipForward()
{
	AudioBuffByteOffset += 96000 * sizeof(float);
	if (AudioBuffByteOffset > audiolen)
	{
		AudioBuffByteOffset -= audiolen;
	}
}

void SkipBackward()
{
	AudioBuffByteOffset -= 96000 * sizeof(float);
	if (AudioBuffByteOffset < 0)
	{
		AudioBuffByteOffset = audiolen + AudioBuffByteOffset;
	}
}

void DeInit()
{
	ma_device_uninit(&device);
	delete[] outputCopy;
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
				if (event.key.key == SDLK_RIGHT)
				{
					SkipForward();
				}
				if (event.key.key == SDLK_LEFT)
				{
					SkipBackward();
				}
			}
		}
	}


	DeInit();
	return 0;
}

