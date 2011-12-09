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

	unsigned channels                 = 1;
	m_Attack.sample_frac_bits  = 8 * bytes_per_sample - 1;
	m_Attack.stage             = GSS_ATTACK;
	m_Attack.type              = GetAudioSectionType(bytes_per_sample, channels);
	m_Loop.sample_frac_bits    = 8 * bytes_per_sample - 1;
	m_Loop.stage               = GSS_LOOP;
	m_Loop.type                = GetAudioSectionType(bytes_per_sample, channels);
	m_Release.sample_frac_bits = 8 * bytes_per_sample - 1;
	m_Release.stage            = GSS_RELEASE;
	m_Release.type             = GetAudioSectionType(bytes_per_sample, channels);

	double trem_freq = 1000.0 / period;
	int sample_freq = 44100;

	unsigned attackSamples  = sample_freq / start_rate;
	unsigned attackSamplesInMem = attackSamples + EXTRA_FRAMES;
	unsigned loopSamples  = sample_freq / trem_freq;
	unsigned loopSamplesInMem = loopSamples + EXTRA_FRAMES;
	unsigned releaseSamples  = sample_freq / stop_rate;
	unsigned releaseSamplesInMem = releaseSamples + EXTRA_FRAMES;

	m_Attack.size = attackSamples * bytes_per_sample * channels;
	m_Attack.alloc_size = attackSamplesInMem * bytes_per_sample * channels;
	m_Attack.data = (unsigned char*)m_pool.Alloc(m_Attack.alloc_size);
	if (m_Attack.data == NULL)
		throw (wxString)_("< out of memory allocating attack");
	m_Attack.sample_rate = sample_freq;
	m_Attack.sample_count = attackSamples;

	m_Loop.size = loopSamples * bytes_per_sample * channels;
	m_Loop.alloc_size = loopSamplesInMem * bytes_per_sample * channels;
	m_Loop.data = (unsigned char*)m_pool.Alloc(m_Loop.alloc_size);
	if (m_Loop.data == NULL)
		throw (wxString)_("< out of memory allocating loop");
	m_Loop.sample_rate = sample_freq;
	m_Loop.sample_count = loopSamples;

	m_Release.size = releaseSamples * bytes_per_sample * channels;
	m_Release.alloc_size = releaseSamplesInMem * bytes_per_sample * channels;
	m_Release.data = (unsigned char*)m_pool.Alloc(m_Release.alloc_size);
	if (m_Release.data == NULL)
		throw (wxString)_("< out of memory allocating release");
	m_Release.sample_rate = sample_freq;
	m_Release.sample_count = releaseSamples;

	double pi = 3.14159265358979323846;
	double trem_amp   = (0x7FF0 * amp_mod_depth / 100);
	double trem_param = 2 * pi  / loopSamples;
	double trem_fade, trem_inc, trem_angle;
	wxInt16 *ptr;
	trem_inc = 1.0 / attackSamples;
	trem_fade = trem_angle = 0.0;
	ptr = (wxInt16*)m_Attack.data;
	for(unsigned i = 0; i < attackSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade += trem_inc;
	}

	ptr = (wxInt16*)m_Loop.data;
	for(unsigned i = 0; i < loopSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle);
		trem_angle += trem_param;
	}

	trem_inc = 1.0 / (double)releaseSamples;
	trem_fade = 1.0 - trem_inc;
	ptr = (wxInt16*)m_Release.data;
	for(unsigned i = 0; i < releaseSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade -= trem_inc;
	}

	memcpy
		(&m_Loop.data[m_Loop.size]
		,&m_Loop.data[0]
		,loopSamplesInMem * bytes_per_sample * channels - m_Loop.size
		);

	memcpy
		(&m_Attack.data[m_Attack.size]
		,&m_Loop.data[0]
		,attackSamplesInMem * bytes_per_sample * channels - m_Attack.size
		);

	memcpy
		(&m_Release.data[m_Release.size]
		,&m_Release.data[m_Release.size-(releaseSamplesInMem * bytes_per_sample * channels - m_Release.size)]
		,releaseSamplesInMem * bytes_per_sample * channels - m_Release.size
		);

	m_Gain = 1.0f;

	ComputeReleaseAlignmentInfo();

}
