/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOSoundAudioSectionAccessor.h"
#include "GOSoundProviderSynthedTrem.h"
#include "GOrgueMemoryPool.h"
#include <wx/wx.h>

GOSoundProviderSynthedTrem::GOSoundProviderSynthedTrem(GOrgueMemoryPool& pool) :
	GOSoundProvider(pool)
{
	m_Gain = 1.0f;
}

short SynthTrem(double amp, double angle)
{
	return (short)(amp * sin(angle));
}

short SynthTrem(double amp, double angle, double fade)
{
	return (short)(fade * amp * sin(angle));
}

void GOSoundProviderSynthedTrem::Create
	(int period
	,int start_rate
	,int stop_rate
	,int amp_mod_depth
	)
{

	ClearData();

	unsigned bytes_per_sample = sizeof(wxInt16);

	unsigned channels            = 1;
	m_Attack.m_SampleFracBits    = 8 * bytes_per_sample - 1;
	m_Attack.m_Stage             = GSS_ATTACK;
	m_Attack.m_Type              = GetAudioSectionType(bytes_per_sample, channels);
	m_Loop.m_SampleFracBits      = 8 * bytes_per_sample - 1;
	m_Loop.m_Stage               = GSS_LOOP;
	m_Loop.m_Type                = GetAudioSectionType(bytes_per_sample, channels);
	m_Release.m_SampleFracBits   = 8 * bytes_per_sample - 1;
	m_Release.m_Stage            = GSS_RELEASE;
	m_Release.m_Type             = GetAudioSectionType(bytes_per_sample, channels);

	double trem_freq = 1000.0 / period;
	int sample_freq = 44100;

	unsigned attackSamples  = sample_freq / start_rate;
	unsigned attackSamplesInMem = attackSamples + EXTRA_FRAMES;
	unsigned loopSamples  = sample_freq / trem_freq;
	unsigned loopSamplesInMem = loopSamples + EXTRA_FRAMES;
	unsigned releaseSamples  = sample_freq / stop_rate;
	unsigned releaseSamplesInMem = releaseSamples + EXTRA_FRAMES;

	m_Attack.m_Size = attackSamples * bytes_per_sample * channels;
	m_Attack.m_AllocSize = attackSamplesInMem * bytes_per_sample * channels;
	m_Attack.m_Data = (unsigned char*)m_pool.Alloc(m_Attack.m_AllocSize);
	if (m_Attack.m_Data == NULL)
		throw (wxString)_("< out of memory allocating attack");
	m_Attack.m_SampleRate = sample_freq;
	m_Attack.m_SampleCount = attackSamples;

	m_Loop.m_Size = loopSamples * bytes_per_sample * channels;
	m_Loop.m_AllocSize = loopSamplesInMem * bytes_per_sample * channels;
	m_Loop.m_Data = (unsigned char*)m_pool.Alloc(m_Loop.m_AllocSize);
	if (m_Loop.m_Data == NULL)
		throw (wxString)_("< out of memory allocating loop");
	m_Loop.m_SampleRate = sample_freq;
	m_Loop.m_SampleCount = loopSamples;

	m_Release.m_Size = releaseSamples * bytes_per_sample * channels;
	m_Release.m_AllocSize = releaseSamplesInMem * bytes_per_sample * channels;
	m_Release.m_Data = (unsigned char*)m_pool.Alloc(m_Release.m_AllocSize);
	if (m_Release.m_Data == NULL)
		throw (wxString)_("< out of memory allocating release");
	m_Release.m_SampleRate = sample_freq;
	m_Release.m_SampleCount = releaseSamples;

	double pi = 3.14159265358979323846;
	double trem_amp   = (0x7FF0 * amp_mod_depth / 100);
	double trem_param = 2 * pi  / loopSamples;
	double trem_fade, trem_inc, trem_angle;
	wxInt16 *ptr;
	trem_inc = 1.0 / attackSamples;
	trem_fade = trem_angle = 0.0;
	ptr = (wxInt16*)m_Attack.m_Data;
	for(unsigned i = 0; i < attackSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade += trem_inc;
	}

	ptr = (wxInt16*)m_Loop.m_Data;
	for(unsigned i = 0; i < loopSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle);
		trem_angle += trem_param;
	}

	trem_inc = 1.0 / (double)releaseSamples;
	trem_fade = 1.0 - trem_inc;
	ptr = (wxInt16*)m_Release.m_Data;
	for(unsigned i = 0; i < releaseSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade -= trem_inc;
	}

	memcpy
		(&m_Loop.m_Data[m_Loop.m_Size]
		,&m_Loop.m_Data[0]
		,loopSamplesInMem * bytes_per_sample * channels - m_Loop.m_Size
		);

	memcpy
		(&m_Attack.m_Data[m_Attack.m_Size]
		,&m_Loop.m_Data[0]
		,attackSamplesInMem * bytes_per_sample * channels - m_Attack.m_Size
		);

	memcpy
		(&m_Release.m_Data[m_Release.m_Size]
		,&m_Release.m_Data[m_Release.m_Size-(releaseSamplesInMem * bytes_per_sample * channels - m_Release.m_Size)]
		,releaseSamplesInMem * bytes_per_sample * channels - m_Release.m_Size
		);

	ComputeReleaseAlignmentInfo();

}
