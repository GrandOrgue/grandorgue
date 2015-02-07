/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOSOUNDOUTPUTWORKITEM_H
#define GOSOUNDOUTPUTWORKITEM_H

#include "GOSoundBufferItem.h"
#include "GOSoundWorkItem.h"
#include "GOLock.h"
#include <vector>

class GOSoundReverb;
class GOrgueSettings;

class GOSoundOutputWorkItem : public GOSoundWorkItem, public GOSoundBufferItem
{
private:
	std::vector<float> m_ScaleFactors;
	std::vector<GOSoundBufferItem*> m_Outputs;
	unsigned m_OutputCount;
	std::vector<float> m_MeterInfo;
	GOSoundReverb* m_Reverb;
	GOMutex m_Mutex;
	unsigned m_Done;

public:
	GOSoundOutputWorkItem(unsigned channels, std::vector<float> scale_factors, unsigned samples_per_buffer);
	~GOSoundOutputWorkItem();

	void SetOutputs(std::vector<GOSoundBufferItem*> outputs);

	unsigned GetGroup();
	unsigned GetCost();
	void Run();
	void Exec();
	void Finish();

	void Clear();
	void Reset();

	void SetupReverb(GOrgueSettings& settings);

	const std::vector<float>& GetMeterInfo();
	void ResetMeterInfo();
};

#endif
