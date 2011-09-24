/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueRtHelpers.h"

void GOrgueRtHelpers::GetDirectSoundConfig(const int latency, unsigned sample_rate, unsigned *nb_buffers, unsigned *buffer_size)
{

	int bufferCalc = (latency * sample_rate) / 1000;

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

void GOrgueRtHelpers::GetAsioSoundConfig(const int latency, unsigned sample_rate, unsigned *nb_buffers, unsigned *buffer_size)
{

	int buffer_calc = (latency * sample_rate) / 1000;

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

void GOrgueRtHelpers::GetJackSoundConfig(const int latency, unsigned sample_rate, unsigned *nb_buffers, unsigned *buffer_size)
{

	*nb_buffers = 1;
	*buffer_size = 1024;

}

void GOrgueRtHelpers::GetUnknownSoundConfig(const int latency, unsigned sample_rate, unsigned *nb_buffers, unsigned *buffer_size)
{

	*buffer_size = 256;
	*nb_buffers = (latency * sample_rate) / (*buffer_size * 1000);
	if (*nb_buffers < 2)
		*nb_buffers = 2;

}

const wxChar* GOrgueRtHelpers::GetApiName(const RtAudio::Api api)
{

	switch (api)
	{

	case RtAudio::LINUX_ALSA:
		return _("Alsa");
	case RtAudio::LINUX_OSS:
		return _("OSS");
	case RtAudio::MACOSX_CORE:
		return _("Core");
	case RtAudio::UNIX_JACK:
		return _("Jack");
	case RtAudio::WINDOWS_ASIO:
		return _("ASIO");
	case RtAudio::WINDOWS_DS:
		return _("DirectSound");
	case RtAudio::UNSPECIFIED:
	default:
		return _("Unknown");

	}

}

void GOrgueRtHelpers::GetBufferConfig
	(const RtAudio::Api rt_api
	,const unsigned latency_ms
	,unsigned sample_rate
	,unsigned *nb_buffers
	,unsigned *buffer_size_samples
	)
{

	switch (rt_api)
	{
	case RtAudio::WINDOWS_DS:
		GetDirectSoundConfig(latency_ms, sample_rate, nb_buffers, buffer_size_samples);
		break;
	case RtAudio::UNIX_JACK:
		GetJackSoundConfig(latency_ms, sample_rate, nb_buffers, buffer_size_samples);
		break;
	case RtAudio::WINDOWS_ASIO:
		GetAsioSoundConfig(latency_ms, sample_rate, nb_buffers, buffer_size_samples);
		break;
	default:
		GetUnknownSoundConfig(latency_ms, sample_rate, nb_buffers, buffer_size_samples);
	}

}

const wxChar* GOrgueRtHelpers::GetAudioFormatName(const int rt_audio_format)
{

	switch (rt_audio_format)
	{

	case RTAUDIO_SINT8:
		return _("8 bit PCM");

	case RTAUDIO_SINT16:
		return _("16 bit PCM");

	case RTAUDIO_SINT24:
		return _("24 bit PCM");

	case RTAUDIO_SINT32:
		return _("32 bit PCM");

	case RTAUDIO_FLOAT32:
		return _("32 bit float");

	case RTAUDIO_FLOAT64:
		return _("64 bit float");

	default:
		return _("none");

	}

}
