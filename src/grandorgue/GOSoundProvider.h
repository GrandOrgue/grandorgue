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

#ifndef GOSOUNDPROVIDER_H_
#define GOSOUNDPROVIDER_H_

#include <wx/wx.h>
#include "GOrgueSoundTypes.h"

class GOSoundProvider
{

private:
	void GetMaxAmplitudeAndDerivative(AUDIO_SECTION& section, int& runningMaxAmplitude, int& runningMaxDerivative);

protected:
	int            ra_amp;
	int            ra_shift;
	AUDIO_SECTION  m_attack;
	AUDIO_SECTION  m_loop;
	AUDIO_SECTION  m_release;
	unsigned int   m_Channels;
	unsigned int   m_SampleRate;
	void ComputeReleaseAlignmentInfo();

public:

	GOSoundProvider();
	~GOSoundProvider();

	void ClearData();

	virtual bool LoadCache(wxInputStream* cache);
	virtual bool SaveCache(wxOutputStream* cache);

	const AUDIO_SECTION* GetLoop() const;
	const AUDIO_SECTION* GetRelease() const;
	const AUDIO_SECTION* GetAttack() const;
	int GetScaleAmplitude() const;
	int GetScaleShift() const;
	int IsOneshot() const;

};

inline
const AUDIO_SECTION* GOSoundProvider::GetLoop() const
{
	return &m_loop;
}

inline
const AUDIO_SECTION* GOSoundProvider::GetRelease() const
{
	return &m_release;
}

inline
const AUDIO_SECTION* GOSoundProvider::GetAttack() const
{
	return &m_attack;
}

inline
int GOSoundProvider::GetScaleAmplitude() const
{
	return ra_amp;
}

inline
int GOSoundProvider::GetScaleShift() const
{
	return ra_shift;
}


#endif /* GOSOUNDPROVIDER_H_ */
