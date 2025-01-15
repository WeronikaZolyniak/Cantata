#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_audio.h>
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device device;
bool beepactive = false;

Uint8* audiobuff;
Uint32 audiolen;
int AudioBuffIndex = 0;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window = nullptr;
SDL_Surface* screenSurface = nullptr;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	if (AudioBuffIndex + frameCount * sizeof(float) * 2 > audiolen)
	{
		int RestOfSoundLength = audiolen - AudioBuffIndex;
		SDL_memcpy(pOutput, audiobuff + AudioBuffIndex, RestOfSoundLength);
		int BegginingLength = (frameCount * sizeof(float) * 2) - RestOfSoundLength;
		//AudioBuffIndex = 0;
		SDL_memcpy(pOutput, audiobuff, BegginingLength);
		AudioBuffIndex = BegginingLength;
		return;
	}

	SDL_memcpy(pOutput, audiobuff + AudioBuffIndex, sizeof(float) * frameCount * 2);
	AudioBuffIndex += sizeof(float) * frameCount * 2;


	/*if (beepactive)
	{
		float temp[4096];
		ma_uint64 framesRead;
		ma_decoder_read_pcm_frames(&beepdecoder, temp, frameCount, &framesRead);

		for (ma_uint64 i = 0; i < framesRead; i++)
		{
			pOutputF32[i] += temp[i];
		}

		if (framesRead < frameCount)
		{
			std::cout << "Finishedbeep\n";
			beepactive = false;
		}
	}*/

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


	ma_device_config deviceconfig = ma_device_config_init(ma_device_type_playback); //automatically sets values to device's native configuration
	/*deviceconfig.playback.format = melodydecoder.outputFormat;
	deviceconfig.playback.channels = melodydecoder.outputChannels;
	deviceconfig.sampleRate = melodydecoder.outputSampleRate;*/
	deviceconfig.dataCallback = data_callback; //setting callback
	//deviceconfig.pUserData = &melodydecoder;


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

	std::cout << "soja";
}

void PlayBeepSound()
{
	if (!beepactive)
	{
		beepactive = true;
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
					std::cout << "Space\n";
					PlayBeepSound();
				}
			}
		}
	}


	DeInit();
	return 0;
}

