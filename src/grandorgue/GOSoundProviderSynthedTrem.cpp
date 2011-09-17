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

#include "GOSoundProviderSynthedTrem.h"

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

	m_Channels = 1;
	if (m_Channels == 1)
	{
		m_Attack.type = AC_UNCOMPRESSED_MONO;
		m_Loop.type = AC_UNCOMPRESSED_MONO;
		m_Release.type = AC_UNCOMPRESSED_MONO;
	}
	else
	{
		m_Attack.type = AC_UNCOMPRESSED_STEREO;
		m_Loop.type = AC_UNCOMPRESSED_STEREO;
		m_Release.type = AC_UNCOMPRESSED_STEREO;
	}

	m_Attack.stage = GSS_ATTACK;
	m_Loop.stage = GSS_LOOP;
	m_Release.stage = GSS_RELEASE;

	double trem_freq = 1000.0 / period;
	int sample_freq = 44100;

	unsigned attackSamples  = sample_freq / start_rate;
	unsigned attackSamplesInMem = attackSamples + BLOCKS_PER_FRAME;
	unsigned loopSamples  = sample_freq / trem_freq;
	unsigned loopSamplesInMem = loopSamples + BLOCKS_PER_FRAME;
	unsigned releaseSamples  = sample_freq / stop_rate;
	unsigned releaseSamplesInMem = releaseSamples + BLOCKS_PER_FRAME;

	m_Attack.size = attackSamples * sizeof(wxInt16) * m_Channels;
	m_Attack.alloc_size = attackSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_Attack.data = (unsigned char*)malloc(m_Attack.alloc_size);
	if (m_Attack.data == NULL)
		throw (wxString)_("< out of memory allocating attack");

	m_Loop.size = loopSamples * sizeof(wxInt16) * m_Channels;
	m_Loop.alloc_size = loopSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_Loop.data = (unsigned char*)malloc(m_Loop.alloc_size);
	if (m_Loop.data == NULL)
		throw (wxString)_("< out of memory allocating loop");

	m_Release.size = releaseSamples * sizeof(wxInt16) * m_Channels;
	m_Release.alloc_size = releaseSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_Release.data = (unsigned char*)malloc(m_Release.alloc_size);
	if (m_Release.data == NULL)
		throw (wxString)_("< out of memory allocating release");

	double pi = 3.14159265358979323846;
	double trem_amp   = (0x7FF0 * amp_mod_depth / 100);
	double trem_param = 2 * pi  / loopSamples;
	double trem_fade, trem_inc, trem_angle;
	short *ptr;
	trem_inc = 1.0 / attackSamples;
	trem_fade = trem_angle = 0.0;
	ptr = (short*)m_Attack.data;
	for(unsigned i = 0; i < attackSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade += trem_inc;
	}

	ptr = (short*)m_Loop.data;
	for(unsigned i = 0; i < loopSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle);
		trem_angle += trem_param;
	}

	trem_inc = 1.0 / (double)releaseSamples;
	trem_fade = 1.0 - trem_inc;
	ptr = (short*)m_Release.data;
	for(unsigned i = 0; i < releaseSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade -= trem_inc;
	}

	memcpy
		(&m_Loop.data[m_Loop.size]
		,&m_Loop.data[0]
		,loopSamplesInMem * sizeof(wxInt16) * m_Channels - m_Loop.size
		);

	memcpy
		(&m_Attack.data[m_Attack.size]
		,&m_Loop.data[0]
		,attackSamplesInMem * sizeof(wxInt16) * m_Channels - m_Attack.size
		);

	memcpy
		(&m_Release.data[m_Release.size]
		,&m_Release.data[m_Release.size-(releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_Release.size)]
		,releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_Release.size
		);

	int amp = 10000;
	m_ScaleShift = 7;
	while (amp > 10000)
	{
		m_ScaleShift--;
		amp >>= 1;
	}
	m_ScaleAmp = (amp << 15) / -10000;

	ComputeReleaseAlignmentInfo();

}
