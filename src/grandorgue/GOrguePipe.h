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

class GOrgueTremulant;
class GOrgueReleaseAlignTable;
class GrandOrgueFile;

class GOrguePipe
{

private:
	GrandOrgueFile* m_organfile;

	void SetOn();
	void SetOff();

	unsigned int m_Channels;
	unsigned int m_SampleRate;

	float pitch;
	GO_SAMPLER* sampler;
	int instances;

	/* states which windchest this pipe belongs to , see GOrgueSound::StartSampler */
	int m_SamplerGroupID;
	int ra_amp;
	int ra_shift;

	AUDIO_SECTION m_attack;
	AUDIO_SECTION m_loop;
	AUDIO_SECTION m_release;

	GOrgueReleaseAlignTable* m_ra_table;

	void GetMaxAmplitudeAndDerivative(AUDIO_SECTION& section, int& runningMaxAmplitude, int& runningMaxDerivative);
	void ComputeReleaseAlignmentInfo();

	wxString m_filename;
	bool m_percussive;
	int m_amplitude;
	GOrguePipe* m_ref;

public:

	~GOrguePipe();
	GOrguePipe(GrandOrgueFile* organfile, wxString filename, bool percussive, int samplerGroupID, int amplitude);

	void Set(bool on);
	void LoadData();
	void CreateTremulant(int period, int startRate, int stopRate, int ampModDepth);

	bool LoadCache(wxInputStream* cache);
	bool SaveCache(wxOutputStream* cache);

	const AUDIO_SECTION* GetLoop();
	const AUDIO_SECTION* GetRelease();

	void FastAbort();

	wxString GetFilename();

};

inline
const AUDIO_SECTION* GOrguePipe::GetLoop()
{
	if (m_ref)
		return m_ref->GetLoop();
	return &m_loop;
}

inline
const AUDIO_SECTION* GOrguePipe::GetRelease()
{
	if (m_ref)
		return m_ref->GetRelease();
	return &m_release;
}

#endif
