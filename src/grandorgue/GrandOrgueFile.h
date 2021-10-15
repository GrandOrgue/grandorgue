/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GRANDORGUEFILE_H
#define GRANDORGUEFILE_H

#include "ptrvector.h"
#include "GOGUIMouseStateTracker.h"
#include "GOrgueBitmapCache.h"
#include "GOrgueCombinationDefinition.h"
#include "GOrgueEventDistributor.h"
#include "GOrgueLabel.h"
#include "GOrgueMainWindowData.h"
#include "GOrgueMemoryPool.h"
#include "GOrgueModel.h"
#include "GOrguePipeConfigTreeNode.h"
#include "GOrgueTimer.h"
#include <wx/hashmap.h>
#include <wx/string.h>
#include <vector>

class GOGUIPanel;
class GOGUIPanelCreator;
class GOrgueArchive;
class GOrgueAudioRecorder;
class GOrgueButton;
class GOrgueCache;
class GOrgueElementCreator;
class GOrgueMidi;
class GOrgueMidiEvent;
class GOrgueMidiPlayer;
class GOrgueMidiRecorder;
class GOrgueOrgan;
class GOrgueProgressDialog;
class GOrgueSetter;
class GOrgueSettings;
class GOrgueTemperament;
class GOrgueDocument;
class GOSoundEngine;
class GOSoundProvider;
class GOSoundRecorder;
class GO_SAMPLER;
typedef struct _GOrgueHashType GOrgueHashType;

class GrandOrgueFile : public GOrgueEventDistributor, private GOrguePipeUpdateCallback, public GOrgueTimer, public GOrgueModel
{
	WX_DECLARE_STRING_HASH_MAP(bool, GOStringBoolMap);

private:
	GOrgueDocument* m_doc;
	wxString m_odf;
	wxString m_ArchiveID;
	wxString m_hash;
	wxString m_path;
	wxString m_CacheFilename;
	wxString m_SettingFilename;
	wxString m_ODFHash;
	bool m_Cacheable;
	GOrgueSetter* m_setter;
	GOrgueAudioRecorder* m_AudioRecorder;
	GOrgueMidiPlayer* m_MidiPlayer;
	GOrgueMidiRecorder* m_MidiRecorder;
	int m_volume;
	bool m_IgnorePitch;
	wxString m_Temperament;

	bool m_b_customized;
	bool m_DivisionalsStoreIntermanualCouplers;
	bool m_DivisionalsStoreIntramanualCouplers;
	bool m_DivisionalsStoreTremulants;
	bool m_GeneralsStoreDivisionalCouplers;
	bool m_CombinationsStoreNonDisplayedDrawstops;

	wxString m_ChurchName;
	wxString m_ChurchAddress;
	wxString m_OrganBuilder;
	wxString m_OrganBuildDate;
	wxString m_OrganComments;
	wxString m_RecordingDetails;
	wxString m_InfoFilename;

	ptr_vector<GOGUIPanel> m_panels;
	ptr_vector<GOGUIPanelCreator> m_panelcreators;
	ptr_vector<GOrgueElementCreator> m_elementcreators;
	ptr_vector<GOrgueArchive> m_archives;
	GOStringBoolMap m_UsedSections;

	GOSoundEngine* m_soundengine;
	GOrgueMidi* m_midi;
	std::vector<bool> m_MidiSamplesetMatch;
	int m_SampleSetId1, m_SampleSetId2;
	GOGUIMouseStateTracker m_MouseState;

	GOrgueMemoryPool m_pool;
	GOrgueBitmapCache m_bitmaps;
	GOrguePipeConfigTreeNode m_PipeConfig;
	GOrgueSettings& m_Settings;
	GOrgueCombinationDefinition m_GeneralTemplate;
	GOrgueLabel m_PitchLabel;
	GOrgueLabel m_TemperamentLabel;
	GOrgueMainWindowData m_MainWindowData;

	void ReadOrganFile(GOrgueConfigReader& cfg);
	GOrgueHashType GenerateCacheHash();
	wxString GenerateSettingFileName();
	wxString GenerateCacheFileName();
	void SetTemperament(const GOrgueTemperament& temperament);
	void PreconfigRecorder();

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateReleaseTruncationLength();
	void UpdateAudioGroup();

	wxString GetOrganHash();

	bool LoadArchive(wxString ID, wxString& name, const wxString& parentID = wxEmptyString);
	void CloseArchives();

public:

	GrandOrgueFile(GOrgueDocument* doc, GOrgueSettings& settings);
	wxString Load(GOrgueProgressDialog* dlg, const GOrgueOrgan& organ, const wxString& cmb = wxEmptyString);
	void LoadCombination(const wxString& cmb);
	bool Save();
	bool Export(const wxString& cmb);
	bool CachePresent();
	bool IsCacheable();
	bool UpdateCache(GOrgueProgressDialog* dlg, bool compress);
	void DeleteCache();
	void DeleteSettings();;
	void Abort();
	void PreparePlayback(GOSoundEngine* engine, GOrgueMidi* midi, GOSoundRecorder* recorder);
	void PrepareRecording();
	void Update();
	void Reset();
	void ProcessMidi(const GOrgueMidiEvent& event);
	void AllNotesOff();
	void Modified();
	GOrgueDocument* GetDocument();
	~GrandOrgueFile(void);

	/* Access to internal ODF objects */
	GOrgueSetter* GetSetter();
	GOGUIPanel* GetPanel(unsigned index);
	unsigned GetPanelCount();
	void AddPanel(GOGUIPanel* panel);
	GOrgueMemoryPool& GetMemoryPool();
	GOrgueSettings& GetSettings();
	GOrgueBitmapCache& GetBitmapCache();
	GOrguePipeConfigNode& GetPipeConfig();
	void SetTemperament(wxString name);
	wxString GetTemperament();
	void MarkSectionInUse(wxString name);

	bool useArchives();
	GOrgueArchive* findArchive(const wxString& name);

	int GetRecorderElementID(wxString name);
	GOrgueCombinationDefinition& GetGeneralTemplate();
	GOrgueLabel* GetPitchLabel();
	GOrgueLabel* GetTemperamentLabel();
	GOrgueMainWindowData* GetMainWindowData();

	void LoadMIDIFile(const wxString& filename);

	/* ODF general properties */
	bool DivisionalsStoreIntermanualCouplers();
	bool DivisionalsStoreIntramanualCouplers();
	bool DivisionalsStoreTremulants();
	bool CombinationsStoreNonDisplayedDrawstops();
	bool GeneralsStoreDivisionalCouplers();

	void SetVolume(int volume);
	int GetVolume();

	void SetIgnorePitch(bool ignorepitch);
	bool GetIgnorePitch();

	GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel = false);
	GOrgueLabel* GetLabel(const wxString& name, bool is_panel = false);
	GOrgueButton* GetButton(const wxString& name, bool is_panel = false);
	
	/* TODO: can somebody figure out what this thing is */
	bool IsCustomized();

	/* Filename of the organ definition used to load */
	const wxString GetODFFilename();
	const wxString GetODFPath();
	const wxString GetOrganPathInfo();
	GOrgueOrgan GetOrganInfo();
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

	GO_SAMPLER* StartSample(const GOSoundProvider *pipe, int sampler_group_id, unsigned audio_group, unsigned velocity, unsigned delay, uint64_t last_stop);
	uint64_t StopSample(const GOSoundProvider *pipe, GO_SAMPLER* handle);
	void SwitchSample(const GOSoundProvider *pipe, GO_SAMPLER* handle);
	void UpdateVelocity(GO_SAMPLER* handle, unsigned velocity);

	void SendMidiMessage(GOrgueMidiEvent& e);
	void SendMidiRecorderMessage(GOrgueMidiEvent& e);
	GOrgueMidi* GetMidi();

	GOGUIMouseStateTracker& GetMouseStateTracker();

	/* For testing only */
	void SetODFPath(wxString path);
};

#endif
