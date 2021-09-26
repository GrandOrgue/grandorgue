/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEWAVPACK_H
#define GORGUEWAVPACK_H

#include "GOrgueBuffer.h"
#include <wavpack/wavpack.h>

class GOrgueWavPack
{
private:
	const GOrgueBuffer<uint8_t>& m_data;
	GOrgueBuffer<uint8_t> m_Samples;
	GOrgueBuffer<uint8_t> m_Wrapper;
	unsigned m_pos;
	unsigned m_OrigDataLen;
	WavpackContext* m_context;

	static WavpackStreamReader m_Reader;

	static uint32_t GetLength (void *id);
	static int32_t ReadBytes (void *id, void *data, int32_t bcount);
	static int PushBackByte (void *id, int c);
	static uint32_t GetPos(void *id);
	static int CanSeek(void *id);
	static int SetPosAbs(void *id, uint32_t pos);
	static int SetPosRel(void *id, int32_t delta, int mode);

	uint32_t GetLength();
	int32_t ReadBytes (void *data, int32_t bcount);
	int PushBackByte (int c);
	uint32_t GetPos();
	int CanSeek();
	int SetPosAbs(uint32_t pos);
	int SetPosRel(int32_t delta, int mode);

public:
	GOrgueWavPack(const GOrgueBuffer<uint8_t>& file);
	~GOrgueWavPack();

	static bool IsWavPack(const GOrgueBuffer<uint8_t>& data);
	bool Unpack();

	GOrgueBuffer<uint8_t> GetSamples();
	GOrgueBuffer<uint8_t> GetWrapper();
	unsigned GetOrigDataLen();
};

#endif
