/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
class GO_SAMPLER;

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
	GO_SAMPLER* m_PlaybackHandle;
	uint64_t m_LastStop;
	int m_SamplerGroupID;

	void InitSoundProvider();
	void ChangeState(bool on);
	void SetupCombinationState();

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(GOrgueHash& hash);
	const wxString& GetLoadTitle();

	void AbortPlayback();
	void StartPlayback();

public:
	GOrgueTremulant(GrandOrgueFile* organfile);
	~GOrgueTremulant();
	void Load(GOrgueConfigReader& cfg, wxString group, int sampler_group_id);
	GOrgueTremulantType GetTremulantType();

	wxString GetMidiType();
};

#endif /* GORGUETREMULANT_H_ */
