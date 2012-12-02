/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDREVERB_H
#define GOSOUNDREVERB_H

class Convproc;
class GOrgueSettings;

class GOSoundReverb
{
private:
	unsigned m_channels;
	Convproc* m_engine;

	void Cleanup();

public:
	GOSoundReverb(unsigned channels);
	virtual ~GOSoundReverb();

	void Reset();
	void Setup(GOrgueSettings& settings);

	void Process(float *output_buffer, unsigned n_frames);
};

#endif
