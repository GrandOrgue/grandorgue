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

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include <wx/wx.h>
#include "GOrgueSoundTypes.h"
#include "GOSoundEngine.h"
#include "GOSoundProviderWave.h"

class GrandOrgueFile;

class GOrguePipe
{

private:
	GrandOrgueFile* m_organfile;
	SAMPLER_HANDLE  m_Sampler;
	float pitch;
	int instances;

	/* states which windchest this pipe belongs to, see GOSoundEngine::StartSampler */
	int m_SamplerGroupID;
	wxString m_filename;
	bool m_percussive;
	int m_amplitude;

	GOrguePipe* m_ref;
	GOSoundProviderWave m_SoundProvider;

	void SetOn();
	void SetOff();
	GOSoundProvider* GetSoundProvider();

public:
	GOrguePipe(GrandOrgueFile* organfile, wxString filename, bool percussive, int samplerGroupID, int amplitude);

	void Set(bool on);
	void LoadData();

	bool LoadCache(wxInputStream* cache);
	bool SaveCache(wxOutputStream* cache);

	const AUDIO_SECTION* GetAttack() const;
	const AUDIO_SECTION* GetLoop() const;
	const AUDIO_SECTION* GetRelease() const;
	int GetScaleAmplitude() const;
	int GetScaleShift() const;
	int GetSamplerGroupID() const;

	void FastAbort();
	wxString GetFilename();

};

inline
int GOrguePipe::GetSamplerGroupID() const
{
	return m_SamplerGroupID;
}

#endif
