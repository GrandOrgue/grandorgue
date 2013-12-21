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

#include "GOSoundReverbPartition.h"

#include <algorithm>
#include <assert.h>

GOSoundReverbPartition::GOSoundReverbPartition(unsigned size, unsigned cnt, unsigned start_pos) :
	m_PartitionSize(size),
	m_PartitionCount(cnt),
	m_fftwTmpReal(0),
	m_fftwTmpComplex(0),
	m_TimeToFreq(0),
	m_FreqToTime(0),
	m_Input(0),
	m_Output(0),
	m_InputPos(start_pos),
	m_InputStartPos(start_pos),
	m_OutputPos(0),
	m_InputHistory(),
	m_IRData(),
	m_InputHistoryPos(0)
{
	m_fftwTmpReal = new float[m_PartitionSize * 2];
	m_fftwTmpComplex = new fftwf_complex[m_PartitionSize + 1];

	m_TimeToFreq = fftwf_plan_dft_r2c_1d(2 * m_PartitionSize, m_fftwTmpReal, m_fftwTmpComplex, FFTW_ESTIMATE);
	m_FreqToTime = fftwf_plan_dft_c2r_1d(2 * m_PartitionSize, m_fftwTmpComplex, m_fftwTmpReal, FFTW_ESTIMATE);
	assert(m_TimeToFreq);
	assert(m_FreqToTime);

	m_Input = new float[m_PartitionSize];
	m_Output = new float[2 * m_PartitionSize];

	for(unsigned i = 0; i < m_PartitionCount; i++)
		m_InputHistory.push_back(new fftwf_complex[m_PartitionSize + 1]);

	for(unsigned i = 0; i < m_PartitionCount; i++)
		m_IRData.push_back(NULL);

	Reset();
}

GOSoundReverbPartition::~GOSoundReverbPartition()
{
	if (m_TimeToFreq)
		fftwf_destroy_plan(m_TimeToFreq);
	if (m_FreqToTime)
		fftwf_destroy_plan(m_FreqToTime);
	if (m_fftwTmpReal)
		delete[] m_fftwTmpReal;
	if (m_fftwTmpComplex)
		delete[] m_fftwTmpComplex;
	if (m_Input)
		delete[] m_Input;
	if (m_Output)
		delete[] m_Output;
}

void GOSoundReverbPartition::Reset()
{
	std::fill(m_Input, m_Input + m_PartitionSize, 0);
	std::fill(m_Output, m_Output + 2 * m_PartitionSize, 0);
	m_InputPos = m_InputStartPos;
	m_OutputPos = 0;
	m_InputHistoryPos = 0;

	for(unsigned i = 0; i < m_InputHistory.size(); i++)
		ZeroComplex(m_InputHistory[i], m_PartitionSize + 1);
}

void GOSoundReverbPartition::ZeroComplex(fftwf_complex* data, unsigned len)
{
	for(unsigned i = 0; i < len; i++)
	{
		data[i][0] = 0;
		data[i][1] = 0;
	}
}


void GOSoundReverbPartition::Process(float* output_buf, const float* input_buf, unsigned len)
{
	unsigned in_pos = 0, out_pos = 0;
	while (in_pos < len)
	{
		while (m_OutputPos < m_PartitionSize && out_pos < len)
			output_buf[out_pos++] += m_Output[m_OutputPos++];

		while (m_InputPos < m_PartitionSize && in_pos < len)
			m_Input[m_InputPos++] = input_buf[in_pos++];

		if (m_InputPos == m_PartitionSize)
		{
			std::copy (m_Input, m_Input + m_PartitionSize, m_fftwTmpReal);
			std::fill(m_fftwTmpReal + m_PartitionSize, m_fftwTmpReal + 2 * m_PartitionSize, 0);
			fftwf_execute(m_TimeToFreq);
			memcpy
				(m_InputHistory[m_InputHistoryPos]
				,m_fftwTmpComplex
				,(m_PartitionSize + 1) * sizeof(fftwf_complex)
				);

			ZeroComplex(m_fftwTmpComplex, m_PartitionSize + 1);
			for(unsigned i = 0, j = m_InputHistoryPos; i < m_IRData.size(); i++)
			{
				if (m_IRData[i])
				{
					const fftwf_complex* ir = m_IRData[i];
					const fftwf_complex* in = m_InputHistory[j];

					for (unsigned k = 0; k <= m_PartitionSize; k++)
					{
						m_fftwTmpComplex[k][0] += in [k][0] * ir [k][0] - in [k][1] * ir [k][1];
						m_fftwTmpComplex[k][1] += in [k][0] * ir [k][1] + in [k][1] * ir [k][0];
					}
				}
				if (j)
					j--;
				else
					j = m_PartitionCount - 1;

			}
			fftwf_execute(m_FreqToTime);

			for(unsigned i = 0; i < m_PartitionSize; i++)
				m_Output[i] = m_Output[i + m_PartitionSize] + m_fftwTmpReal[i];
			std::copy (m_fftwTmpReal + m_PartitionSize, m_fftwTmpReal + 2 * m_PartitionSize, m_Output + m_PartitionSize);

			m_InputHistoryPos = (m_InputHistoryPos + 1) % m_PartitionCount;
			m_OutputPos = 0;
			m_InputPos = 0;
		}
	}
}

unsigned GOSoundReverbPartition::GetLength()
{
	return m_PartitionSize * m_PartitionCount;
}


void GOSoundReverbPartition::AddIR(const float* data, unsigned pos, unsigned len, unsigned offset)
{
	for(unsigned i = 0; i < m_PartitionCount; i++)
	{
		unsigned minpos = offset + i * m_PartitionSize;
		unsigned maxpos = offset + (i + 1) * m_PartitionSize;
		if (pos + len < minpos || pos >= maxpos)
			continue;
		std::fill(m_fftwTmpReal, m_fftwTmpReal + 2 * m_PartitionSize, 0);
		unsigned startpos = minpos < pos ? pos : minpos;
		unsigned endpos = pos + len > maxpos ? maxpos : pos + len;
		float factor = 0.5 / m_PartitionSize;
		for(unsigned j = startpos; j < endpos; j++)
			m_fftwTmpReal[j - minpos] = factor * data[j - pos];
		fftwf_execute(m_TimeToFreq);
		if (!m_IRData[i])
		{
			m_IRData[i] = new fftwf_complex[m_PartitionSize + 1];
			ZeroComplex(m_IRData[i], m_PartitionSize + 1);
		}
		for(unsigned j = 0; j < m_PartitionSize; j++)
		{
			m_IRData[i][j][0] += m_fftwTmpComplex[j][0];
			m_IRData[i][j][1] += m_fftwTmpComplex[j][1];
		}
	}
}
