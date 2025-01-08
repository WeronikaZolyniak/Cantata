#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_audio.h>
#include <iostream>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device device;
ma_decoder decoder;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
	ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
	if (pDecoder == NULL) { return; }
	ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);
}

void Init()
{
	if (ma_decoder_init_file("melody.wav", NULL, &decoder))
	{
		std::cout << "miniaudio decoder file could not be initialized\n";
		exit(-1);
	}

	ma_device_config deviceconfig = ma_device_config_init(ma_device_type_playback); //automatically sets values to device's native configuration
	deviceconfig.playback.format = decoder.outputFormat;
	deviceconfig.playback.channels = decoder.outputChannels;
	deviceconfig.sampleRate = decoder.outputSampleRate;
	deviceconfig.dataCallback = data_callback; //setting callback
	deviceconfig.pUserData = &decoder;


	if (ma_device_init(NULL, &deviceconfig, &device) != MA_SUCCESS)
	{
		std::cout << "miniaudio device could not be initialized\n";
		exit(-1);
	}	
}

void DeInit()
{
	ma_device_uninit(&device);
	ma_decoder_uninit(&decoder);
}

int main(int argc, char* args[])
{
	Init();
	ma_device_start(&device);

	std::cout << "Press enter to quit\n";
	getchar();
	DeInit();
	return 0;
}

