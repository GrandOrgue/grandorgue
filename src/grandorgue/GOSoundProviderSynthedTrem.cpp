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

void GOSoundProviderSynthedTrem::Create(int period, int startRate, int stopRate, int ampModDepth)
{
	m_Channels = 1;
	if (m_Channels == 1)

	{
		m_attack.type = AC_UNCOMPRESSED_MONO;
		m_loop.type = AC_UNCOMPRESSED_MONO;
		m_release.type = AC_UNCOMPRESSED_MONO;
	}
	else
	{
		m_attack.type = AC_UNCOMPRESSED_STEREO;
		m_loop.type = AC_UNCOMPRESSED_STEREO;
		m_release.type = AC_UNCOMPRESSED_STEREO;
	}

	m_attack.stage = GSS_ATTACK;
	m_loop.stage = GSS_LOOP;
	m_release.stage = GSS_RELEASE;

	double trem_freq = 1000.0 / period;
	int sample_freq = 44100;

	unsigned attackSamples  = sample_freq / startRate;
	unsigned attackSamplesInMem = attackSamples + BLOCKS_PER_FRAME;
	unsigned loopSamples  = sample_freq / trem_freq;
	unsigned loopSamplesInMem = loopSamples + BLOCKS_PER_FRAME;
	unsigned releaseSamples  = sample_freq / stopRate;
	unsigned releaseSamplesInMem = releaseSamples + BLOCKS_PER_FRAME;

	m_attack.size = attackSamples * sizeof(wxInt16) * m_Channels;
	m_attack.alloc_size = attackSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_attack.data = (unsigned char*)malloc(m_attack.alloc_size);
	if (m_attack.data == NULL)
		throw (wxString)_("< out of memory allocating attack");

	m_loop.size = loopSamples * sizeof(wxInt16) * m_Channels;
	m_loop.alloc_size = loopSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_loop.data = (unsigned char*)malloc(m_loop.alloc_size);
	if (m_loop.data == NULL)
		throw (wxString)_("< out of memory allocating loop");

	m_release.size = releaseSamples * sizeof(wxInt16) * m_Channels;
	m_release.alloc_size = releaseSamplesInMem * sizeof(wxInt16) * m_Channels;
	m_release.data = (unsigned char*)malloc(m_release.alloc_size);
	if (m_release.data == NULL)
		throw (wxString)_("< out of memory allocating release");

	double pi = 3.14159265358979323846;
	double trem_amp   = (0x7FF0 * ampModDepth / 100);
	double trem_param = 2 * pi  / loopSamples;
	double trem_fade, trem_inc, trem_angle;
	short *ptr;
	trem_inc = 1.0 / attackSamples;
	trem_fade = trem_angle = 0.0;
	ptr = (short*)m_attack.data;
	for(unsigned i = 0; i < attackSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade += trem_inc;
	}

	ptr = (short*)m_loop.data;
	for(unsigned i = 0; i < loopSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle);
		trem_angle += trem_param;
	}

	trem_inc = 1.0 / (double)releaseSamples;
	trem_fade = 1.0 - trem_inc;
	ptr = (short*)m_release.data;
	for(unsigned i = 0; i < releaseSamples; i++)
	{
		ptr[i] = SynthTrem(trem_amp, trem_angle, trem_fade);
		trem_angle += trem_param * trem_fade;
		trem_fade -= trem_inc;
	}

	memcpy(&m_loop.data[m_loop.size],
	       &m_loop.data[0],
	       loopSamplesInMem * sizeof(wxInt16) * m_Channels - m_loop.size);

	memcpy(&m_attack.data[m_attack.size],
	       &m_loop.data[0],
	       attackSamplesInMem * sizeof(wxInt16) * m_Channels - m_attack.size);

	memcpy(&m_release.data[m_release.size],
	       &m_release.data[m_release.size-(releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_release.size)],
	       releaseSamplesInMem * sizeof(wxInt16) * m_Channels - m_release.size);

	int amp = 10000;
	ra_shift = 7;
	while (amp > 10000)
	{
		ra_shift--;
		amp >>= 1;
	}
	ra_amp = (amp << 15) / -10000;

	ComputeReleaseAlignmentInfo();

}
