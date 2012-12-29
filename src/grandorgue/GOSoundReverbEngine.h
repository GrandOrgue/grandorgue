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

#ifndef GOSOUNDREVERBENGINE_H
#define GOSOUNDREVERBENGINE_H

#include "ptrvector.h"

class GOSoundReverbPartition;

class GOSoundReverbEngine
{
private:
	ptr_vector<GOSoundReverbPartition> m_Partitions;

public:
	GOSoundReverbEngine(unsigned samples_per_buffer);
	~GOSoundReverbEngine();

	void Reset();
	void Process(float* output_buf, const float* input_buf, unsigned len);
	void AddIR(const float* data, unsigned pos, unsigned len);
};

#endif
