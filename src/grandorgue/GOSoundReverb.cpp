/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOSoundReverb.h"

#include "GOSoundResample.h"
#include "GOrgueSettings.h"
#include "GOrgueWave.h"
#include "contrib/zita-convolver.h"
#include <wx/intl.h>
#include <wx/log.h>
#include <algorithm>

GOSoundReverb::GOSoundReverb(unsigned channels) :
	m_channels(channels),
	m_engine(0)
{
}

GOSoundReverb::~GOSoundReverb()
{
	Cleanup();
}

void GOSoundReverb::Cleanup()
{
	if (m_engine)
	{
		m_engine->stop_process();
		m_engine->cleanup();
		delete m_engine;
		m_engine = NULL;
	}
}

void GOSoundReverb::Setup(GOrgueSettings& settings)
{
	Cleanup();

	if (!settings.GetReverbEnabled())
		return;

	m_engine = new Convproc();
	unsigned val = settings.GetSamplesPerBuffer();
	if (val < Convproc::MINPART)
		val = Convproc::MINPART;
	if (val > Convproc::MAXPART)
		val = Convproc::MAXPART;
	float* data = NULL;
	unsigned len = 0;
	try
	{
		if (m_engine->configure(m_channels, m_channels, 1000000, settings.GetSamplesPerBuffer(), val, Convproc::MAXPART))
			throw (wxString)_("Invalid reverb configuration (samples per buffer)");

		GOrgueWave wav;
		unsigned block = 0x4000;
		unsigned offset = settings.GetReverbStartOffset();
		float gain = settings.GetReverbGain();

		wav.Open(settings.GetReverbFile());
		if (offset > wav.GetLength())
			throw (wxString)_("Invalid reverb start offset");
		len = wav.GetLength();
		data = (float*)malloc(sizeof(float)*len);
		if (!data)
			throw (wxString)_("Out of memory");
		wav.ReadSamples(data, GOrgueWave::SF_IEEE_FLOAT, wav.GetSampleRate(), -settings.GetReverbChannel());
		for(unsigned i = 0; i < len; i++)
			data[i] *= gain;
		if (len >= offset + settings.GetReverbLen() && settings.GetReverbLen())
			len = offset + settings.GetReverbLen();
		if (wav.GetSampleRate() != settings.GetSampleRate())
		{
			float* new_data = resample_block(data, len, wav.GetSampleRate(), settings.GetSampleRate());
			if (!new_data)
				throw (wxString)_("Resampling failed");
			free(data);
			data = new_data;
			offset = (offset * settings.GetSampleRate()) / (float)wav.GetSampleRate();
		}
		unsigned delay = (settings.GetSampleRate() * settings.GetReverbDelay()) / 1000;
		for(unsigned i = 0; i < m_channels; i++)
		{
			float* d = data + offset;
			unsigned l = len - offset;
			float g = 1;
			if  (settings.GetReverbDirect())
				m_engine->impdata_create(i, i, 0, &g, 0, 1);
			for(unsigned j = 0; j < l; j+= block)
			{
				m_engine->impdata_create(i, i, 1, d + j, delay + j, delay + j + std::min(l - j, block));
			}
		}
		wav.Close();
		m_engine->start_process(0, 0);
	}
	catch(wxString error)
	{
		wxLogError(_("Load error: %s"), error.c_str());
		delete m_engine;
		m_engine = NULL;
	}
	if (data)
		free(data);
}

void GOSoundReverb::Reset()
{
	if (m_engine)
		m_engine->reset();
}

void GOSoundReverb::Process(float *output_buffer, unsigned n_frames)
{
	if (!m_engine)
		return;

	for(unsigned i = 0; i < m_channels; i++)
	{
		float* ptr = m_engine->inpdata(i);
		for(unsigned j = 0; j < n_frames; j++)
			ptr[j] = output_buffer[i + m_channels * j];
	}

	m_engine->process();

	for(unsigned i = 0; i < m_channels; i++)
	{
		float* ptr = m_engine->outdata(i);
		for(unsigned j = 0; j < n_frames; j++)
			output_buffer[i + m_channels * j] = ptr[j];
	}
}
