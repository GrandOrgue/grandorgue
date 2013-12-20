/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEWAVPACK_H
#define GORGUEWAVPACK_H

#include "wavpack.h"

class GOrgueWavPack
{
private:
	const char* m_data;
	unsigned m_length;
	char* m_Samples;
	unsigned m_SamplesLen;
	char* m_Wrapper;
	unsigned m_WrapperLen;
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
	GOrgueWavPack(const char* data, unsigned length);
	~GOrgueWavPack();

	bool IsWavPack();
	bool Unpack();

	void GetSamples(char*& data, unsigned& len);
	void GetWrapper(char*& data, unsigned& len);
	unsigned GetOrigDataLen();
};

#endif
