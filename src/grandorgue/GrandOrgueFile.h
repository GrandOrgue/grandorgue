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

#ifndef GRANDORGUEFILE_H
#define GRANDORGUEFILE_H

#include "ptrvector.h"
#include "GOrgueBitmapCache.h"
#include "GOrgueCombinationDefinition.h"
#include "GOrgueLabel.h"
#include "GOrgueMemoryPool.h"
#include "GOrguePipeConfig.h"
#include <wx/hashmap.h>
#include <wx/string.h>
#include <vector>

class GOGUIPanel;
class GOrgueCache;
class GOrgueCacheObject;
class GOrgueDivisionalCoupler;
class GOrgueEnclosure;
class GOrgueEventHandler;
class GOrgueGeneral;
class GOrgueManual;
class GOrgueMidi;
class GOrgueMidiEvent;
class GOrgueMidiListener;
class GOrguePiston;
class GOrgueProgressDialog;
class GOrguePushbutton;
class GOrgueRank;
class GOrgueSaveableObject;
class GOrgueSetter;
class GOrgueSettings;
class GOrgueSwitch;
class GOrgueTemperament;
class GOrgueTremulant;
class GOrgueWindchest;
class GOrgueDocument;
class GOSoundEngine;
class GOSoundProvider;

typedef struct GO_SAMPLER_T* SAMPLER_HANDLE;

class GrandOrgueFile : public GOrguePipeUpdateCallback
{
	WX_DECLARE_STRING_HASH_MAP(bool, GOStringBoolMap);

private:
	GOrgueDocument* m_doc;
	wxString m_odf;
	wxString m_path;
	wxString m_CacheFilename;
	wxString m_SettingFilename;
	wxString m_ODFHash;
	bool m_Cacheable;
	GOrgueSetter* m_setter;
	int m_volume;
	bool m_IgnorePitch;
	wxString m_Temperament;

	bool m_b_customized;
	bool m_DivisionalsStoreIntermanualCouplers;
	bool m_DivisionalsStoreIntramanualCouplers;
	bool m_DivisionalsStoreTremulants;
	bool m_GeneralsStoreDivisionalCouplers;
	bool m_CombinationsStoreNonDisplayedDrawstops;

	unsigned m_FirstManual;

	wxString m_ChurchName;
	wxString m_ChurchAddress;
	wxString m_OrganBuilder;
	wxString m_OrganBuildDate;
	wxString m_OrganComments;
	wxString m_RecordingDetails;
	wxString m_InfoFilename;
	unsigned m_RankCount;
	unsigned m_ODFManualCount;

	ptr_vector<GOrgueEnclosure> m_enclosure;
	ptr_vector<GOrgueTremulant> m_tremulant;
	ptr_vector<GOrgueWindchest> m_windchest;
	ptr_vector<GOrguePiston> m_piston;
	ptr_vector<GOrgueGeneral> m_general;
	ptr_vector<GOrgueDivisionalCoupler> m_divisionalcoupler;
	ptr_vector<GOrgueSwitch> m_switches;
	ptr_vector<GOrgueRank> m_ranks;
	ptr_vector<GOrgueManual> m_manual;
	ptr_vector<GOGUIPanel> m_panels;
	std::vector<GOrgueEventHandler*> m_handler;
	std::vector<GOrgueCacheObject*> m_CacheObjects;
	std::vector<GOrgueSaveableObject*> m_SaveableObjects;
	GOStringBoolMap m_UsedSections;

	GOSoundEngine* m_soundengine;
	GOrgueMidi* m_midi;

	GOrgueMemoryPool m_pool;
	GOrgueBitmapCache m_bitmaps;
	GOrguePipeConfig m_PipeConfig;
	GOrgueSettings& m_Settings;
	GOrgueCombinationDefinition m_GeneralTemplate;
	GOrgueLabel m_PitchLabel;
	GOrgueLabel m_TemperamentLabel;

	void ReadOrganFile(GOrgueConfigReader& cfg);
	void ReadCombinations(GOrgueConfigReader& cfg);
	void GenerateCacheHash(unsigned char hash[20]);
	wxString GenerateSettingFileName();
	wxString GenerateCacheFileName();
	void SetTemperament(const GOrgueTemperament& temperament);
	void ResolveReferences();

public:

	GrandOrgueFile(GOrgueDocument* doc, GOrgueSettings& settings);
	wxString Load(GOrgueProgressDialog* dlg, const wxString& odf, const wxString& cmb = wxEmptyString);
	void LoadCombination(const wxString& cmb);
	bool Save();
	bool Export(const wxString& cmb);
	bool CachePresent();
	bool IsCacheable();
	bool UpdateCache(GOrgueProgressDialog* dlg, bool compress);
	void DeleteCache();
	void DeleteSettings();;
	void Abort();
	void PreparePlayback(GOSoundEngine* engine, GOrgueMidi* midi);
	void PrepareRecording();
	void Update();
	void Reset();
	void ProcessMidi(const GOrgueMidiEvent& event);
	void HandleKey(int key);
	void AllNotesOff();
	void ControlChanged(void* control);
	void UpdateTremulant(GOrgueTremulant* tremulant);
	void UpdateVolume();
	void Modified();
	void RegisterEventHandler(GOrgueEventHandler* handler);
	void RegisterCacheObject(GOrgueCacheObject* obj);
	void RegisterSaveableObject(GOrgueSaveableObject* obj);
	GOrgueDocument* GetDocument();
	~GrandOrgueFile(void);

	/* Access to internal ODF objects */
	unsigned GetManualAndPedalCount();
	unsigned GetODFManualCount();
	unsigned GetFirstManualIndex();
	GOrgueManual* GetManual(unsigned index);
	unsigned GetTremulantCount();
	GOrgueRank* GetRank(unsigned index);
	unsigned GetRankCount();
	void AddRank(GOrgueRank* rank);
	GOrgueTremulant* GetTremulant(unsigned index);
	unsigned GetDivisionalCouplerCount();
	GOrgueDivisionalCoupler* GetDivisionalCoupler(unsigned index);
	unsigned GetGeneralCount();
	GOrgueGeneral* GetGeneral(unsigned index);
	unsigned GetSwitchCount();
	GOrgueSwitch* GetSwitch(unsigned index);
	unsigned GetNumberOfReversiblePistons();
	GOrguePiston* GetPiston(unsigned index);
	GOrgueSetter* GetSetter();
	unsigned AddWindchest(GOrgueWindchest* windchest);
	GOrgueWindchest* GetWindchest(unsigned index);
	unsigned GetWindchestGroupCount();
	GOrgueEnclosure* GetEnclosure(unsigned index);
	unsigned GetEnclosureCount();
	unsigned AddEnclosure(GOrgueEnclosure* enclosure);
	GOGUIPanel* GetPanel(unsigned index);
	unsigned GetPanelCount();
	GOrgueMemoryPool& GetMemoryPool();
	GOrgueSettings& GetSettings();
	GOrgueBitmapCache& GetBitmapCache();
	GOrguePipeConfig& GetPipeConfig();
	void SetTemperament(wxString name);
	wxString GetTemperament();
	void MarkSectionInUse(wxString name);

	int GetRecorderElementID(wxString name);
	GOrgueCombinationDefinition& GetGeneralTemplate();
	GOrgueLabel* GetPitchLabel();
	GOrgueLabel* GetTemperamentLabel();

	/* ODF general properties */
	bool DivisionalsStoreIntermanualCouplers();
	bool DivisionalsStoreIntramanualCouplers();
	bool DivisionalsStoreTremulants();
	bool CombinationsStoreNonDisplayedDrawstops();
	bool GeneralsStoreDivisionalCouplers();

	void SetVolume(int volume);
	int GetVolume();

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateAudioGroup();
	
	void SetIgnorePitch(bool ignorepitch);
	bool GetIgnorePitch();
	
	/* TODO: can somebody figure out what this thing is */
	bool IsCustomized();

	/* Filename of the organ definition used to load */
	const wxString GetODFFilename();
	const wxString GetODFPath();
	const wxString GetSettingFilename();
	const wxString GetCacheFilename();

	/* Organ and Building general information */
	const wxString& GetChurchName();
	const wxString& GetChurchAddress();
	const wxString& GetOrganBuilder();
	const wxString& GetOrganBuildDate();
	const wxString& GetOrganComments();
	const wxString& GetRecordingDetails();
	const wxString& GetInfoFilename();

	SAMPLER_HANDLE StartSample(const GOSoundProvider *pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay);
	void StopSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle);
	void SwitchSample(const GOSoundProvider *pipe, SAMPLER_HANDLE handle);
	void UpdateVelocity(SAMPLER_HANDLE handle, unsigned velocity);

	void SendMidiMessage(GOrgueMidiEvent& e);
	void SendMidiRecorderMessage(GOrgueMidiEvent& e);
	void AddMidiListener(GOrgueMidiListener* listener);
};

#endif
