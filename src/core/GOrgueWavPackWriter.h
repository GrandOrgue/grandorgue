/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
