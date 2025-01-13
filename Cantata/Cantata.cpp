#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_audio.h>
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device device;
ma_decoder melodydecoder;
ma_decoder beepdecoder;
bool beepactive = false;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Window* window = nullptr;
SDL_Surface* screenSurface = nullptr;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	float* pOutputF32 = (float*)pOutput;
	ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
	if (pDecoder == NULL) { return; }

	ma_decoder_read_pcm_frames(pDecoder, pOutputF32, frameCount, NULL);

	if (beepactive)
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
	}

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

	
	if (ma_decoder_init_file("melody.wav", NULL, &melodydecoder) != MA_SUCCESS)
	{
		std::cout << "miniaudio decoder file(s) could not be initialized\n";
		exit(-1);
	}

	ma_decoder_config decoder_config = ma_decoder_config_init(melodydecoder.outputFormat, melodydecoder.outputChannels, melodydecoder.outputSampleRate);
	if (ma_decoder_init_file("beep.wav", &decoder_config, &beepdecoder) != MA_SUCCESS)
	{
		std::cout << "miniaudio decoder file(s) could not be initialized\n";
		exit(-1);
	}

	ma_device_config deviceconfig = ma_device_config_init(ma_device_type_playback); //automatically sets values to device's native configuration
	deviceconfig.playback.format = melodydecoder.outputFormat;
	deviceconfig.playback.channels = melodydecoder.outputChannels;
	deviceconfig.sampleRate = melodydecoder.outputSampleRate;
	deviceconfig.dataCallback = data_callback; //setting callback
	deviceconfig.pUserData = &melodydecoder;


	if (ma_device_init(NULL, &deviceconfig, &device) != MA_SUCCESS)
	{
		std::cout << "miniaudio melody device could not be initialized\n";
		exit(-1);
	}
}

void PlayBeepSound()
{
	if (!beepactive)
	{
		beepactive = true;
		ma_decoder_seek_to_pcm_frame(&beepdecoder, 0);
	}
	
}

void DeInit()
{
	ma_device_uninit(&device);
	ma_decoder_uninit(&melodydecoder);
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

