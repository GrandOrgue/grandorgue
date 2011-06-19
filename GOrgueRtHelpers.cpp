/*
 * GOrgueRtHelpers.cpp
 *
 *  Created on: Jun 19, 2011
 *      Author: nick
 */

#include "GOrgueRtHelpers.h"

void GOrgueRtHelpers::GetDirectSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size)
{

	int bufferCalc = (latency * 44100) / 1000;

	*nb_buffers = 2;

	if (bufferCalc <= 256)
	{
		*buffer_size = 128;
		return;
	}

	if (bufferCalc <= 512)
	{
		*buffer_size = 256;
		return;
	}

	if (bufferCalc <= 3072)
	{
		*buffer_size = 512;
		*nb_buffers = bufferCalc / 512;
		if (bufferCalc % 512)
			(*nb_buffers)++;
		return;
	}

	*buffer_size = 1024;
	*nb_buffers = bufferCalc / 1024;
	if (bufferCalc % 1024)
		(*nb_buffers)++;

}

void GOrgueRtHelpers::GetAsioSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size)
{

	int buffer_calc = (latency * 44100) / 1000;

	*nb_buffers = 2;

	if (buffer_calc <= 256)
		*buffer_size = 128;
	else if (buffer_calc <= 512)
		*buffer_size = 256;
	else if (buffer_calc <= 1024)
		*buffer_size = 512;
	else
		*buffer_size = 1024;

}

void GOrgueRtHelpers::GetJackSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size)
{

	*nb_buffers = 1;
	*buffer_size = 1024;

}

void GOrgueRtHelpers::GetUnknownSoundConfig(const int latency, unsigned *nb_buffers, unsigned *buffer_size)
{

	*buffer_size = 256;
	*nb_buffers = (latency * 44100) / (*buffer_size * 1000);
	if (*nb_buffers < 2)
		*nb_buffers = 2;

}

const wxChar* GOrgueRtHelpers::GetApiName(const RtAudio::Api api)
{

	switch (api)
	{

	case RtAudio::LINUX_ALSA:
		return wxT("Alsa");
	case RtAudio::LINUX_OSS:
		return wxT("OSS");
	case RtAudio::MACOSX_CORE:
		return wxT("Core");
	case RtAudio::UNIX_JACK:
		return wxT("Jack");
	case RtAudio::WINDOWS_ASIO:
		return wxT("ASIO");
	case RtAudio::WINDOWS_DS:
		return wxT("DirectSound");
	case RtAudio::UNSPECIFIED:
	default:
		return wxT("Unknown");

	}

}

void GOrgueRtHelpers::GetBufferConfig
	(const RtAudio::Api rt_api
	,const unsigned latency_ms
	,unsigned *nb_buffers
	,unsigned *buffer_size_samples
	)
{

	switch (rt_api)
	{
	case RtAudio::WINDOWS_DS:
		GetDirectSoundConfig(latency_ms, nb_buffers, buffer_size_samples);
		break;
	case RtAudio::UNIX_JACK:
		GetJackSoundConfig(latency_ms, nb_buffers, buffer_size_samples);
		break;
	case RtAudio::WINDOWS_ASIO:
		GetAsioSoundConfig(latency_ms, nb_buffers, buffer_size_samples);
		break;
	default:
		GetUnknownSoundConfig(latency_ms, nb_buffers, buffer_size_samples);
	}

}

const wxChar* GOrgueRtHelpers::GetAudioFormatName(const int rt_audio_format)
{

	switch (rt_audio_format)
	{

	case RTAUDIO_SINT8:
		return wxT("8 bit PCM");

	case RTAUDIO_SINT16:
		return wxT("16 bit PCM");

	case RTAUDIO_SINT24:
		return wxT("24 bit PCM");

	case RTAUDIO_SINT32:
		return wxT("32 bit PCM");

	case RTAUDIO_FLOAT32:
		return wxT("32 bit float");

	case RTAUDIO_FLOAT64:
		return wxT("64 bit float");

	default:
		return wxT("none");

	}

}
