/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include <wx/wx.h>
#include "GOSoundProviderWave.h"

class GrandOrgueFile;
class GOrgueCache;
class GOrgueCacheWriter;
class IniFileConfig;
typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

class GOrguePipe
{

private:
	GrandOrgueFile* m_OrganFile;
	SAMPLER_HANDLE  m_Sampler;
	int m_Instances;

	/* states which windchest this pipe belongs to, see GOSoundEngine::StartSampler */
	int m_SamplerGroupID;
	wxString m_Filename;
	bool m_Percussive;
	int m_Amplitude;
	GOrguePipe* m_Reference;
	GOSoundProviderWave m_SoundProvider;

	void SetOn();
	void SetOff();
	GOSoundProvider* GetSoundProvider();

public:
	GOrguePipe(GrandOrgueFile* organfile, bool percussive, int sampler_group_id, int amplitude);
	void Load(IniFileConfig& cfg, wxString group, wxString prefix);
	void Save(IniFileConfig& cfg, bool prefix);
	void Set(bool on);
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void FastAbort();
	wxString GetFilename();

};

#endif
