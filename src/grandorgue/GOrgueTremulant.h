/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUETREMULANT_H
#define GORGUETREMULANT_H

#include "GOrgueCacheObject.h"
#include "GOrgueDrawStop.h"
#include <wx/string.h>

class GOSoundProvider;
class GOrgueConfigReader;
class GOrgueConfigWriter;
struct IniFileEnumEntry;
typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

typedef enum { GOSynthTrem, GOWavTrem } GOrgueTremulantType;

class GOrgueTremulant : public GOrgueDrawstop, private GOrgueCacheObject
{
private:
	static const struct IniFileEnumEntry m_tremulant_types[];
	GOrgueTremulantType m_TremulantType;
	int m_Period;
	int m_StartRate;
	int m_StopRate;
	int m_AmpModDepth;
	GOSoundProvider* m_TremProvider;
	SAMPLER_HANDLE m_PlaybackHandle;
	int m_SamplerGroupID;

	void InitSoundProvider();
	void ChangeState(bool on);

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(SHA_CTX& ctx);
	const wxString& GetLoadTitle();

public:
	GOrgueTremulant(GrandOrgueFile* organfile);
	~GOrgueTremulant();
	void Load(GOrgueConfigReader& cfg, wxString group, int sampler_group_id);
	void Save(GOrgueConfigWriter& cfg);
	void Abort();
	void PreparePlayback();
	GOrgueTremulantType GetTremulantType();
};

#endif /* GORGUETREMULANT_H_ */
