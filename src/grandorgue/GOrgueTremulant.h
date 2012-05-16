/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUETREMULANT_H
#define GORGUETREMULANT_H

#include <wx/wx.h>
#include "GOrgueCacheObject.h"
#include "GOrgueDrawStop.h"
#include "GOSoundProviderSynthedTrem.h"

class GOrgueConfigWriter;
class IniFileConfig;
struct IniFileEnumEntry;
typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

typedef enum { GOSynthTrem, GOWavTrem } GOrgueTremulantType;

class GOrgueTremulant : public GOrgueDrawstop, public GOrgueCacheObject
{
private:
	static const struct IniFileEnumEntry m_tremulant_types[];
	GOrgueTremulantType m_TremulantType;
	int m_Period;
	int m_StartRate;
	int m_StopRate;
	int m_AmpModDepth;
	GOSoundProviderSynthedTrem m_TremProvider;
	SAMPLER_HANDLE m_PlaybackHandle;
	int m_SamplerGroupID;

public:
	GOrgueTremulant(GrandOrgueFile* organfile);
	~GOrgueTremulant();
	void Load(IniFileConfig& cfg, wxString group, int sampler_group_id);
	void Save(GOrgueConfigWriter& cfg);
	void InitSoundProvider();
	void Set(bool on);
	void Abort();
	void PreparePlayback();
	GOrgueTremulantType GetTremulantType();

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(SHA_CTX& ctx);
	wxString GetLoadTitle();
};

#endif /* GORGUETREMULANT_H_ */
