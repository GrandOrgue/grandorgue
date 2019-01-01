/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEWAVPACKWRITER_H
#define GORGUEWAVPACKWRITER_H

#include "GOrgueBuffer.h"
#include <wavpack/wavpack.h>

class GOrgueWavPackWriter
{
private:
	GOrgueBuffer<uint8_t> m_Output;
	WavpackContext* m_Context;

	bool Write(void* data, int32_t count);
	static int WriteCallback(void *id, void *data, int32_t bcount);
	bool Close();

public:
	GOrgueWavPackWriter();
	~GOrgueWavPackWriter();

	bool Init(unsigned channels, unsigned bitsPerSample, unsigned bytesPerSample, unsigned sampleRate, unsigned sampleCount);
	bool AddWrapper(GOrgueBuffer<uint8_t>& header);
	bool AddSampleData(GOrgueBuffer<int32_t>& sampleData);
	bool GetResult(GOrgueBuffer<uint8_t>& result);
};

#endif
