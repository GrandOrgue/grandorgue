/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANCONTROLLER_H
#define GOORGANCONTROLLER_H

#include <wx/hashmap.h>
#include <wx/string.h>

#include <vector>

#include "ptrvector.h"

#include "combinations/model/GOCombinationDefinition.h"
#include "control/GOEventDistributor.h"
#include "control/GOLabelControl.h"
#include "gui/GOGUIMouseState.h"
#include "loader/GOFileStore.h"
#include "model/GOModificationListener.h"
#include "model/GOOrganModel.h"

#include "GOBitmapCache.h"
#include "GOMainWindowData.h"
#include "GOMemoryPool.h"
#include "GOTimer.h"

class GOGUIPanel;
class GOGUIPanelCreator;
class GOArchive;
class GOAudioRecorder;
class GOButtonControl;
class GOCache;
class GODivisionalSetter;
class GOElementCreator;
class GOMidi;
class GOMidiEvent;
class GOMidiPlayer;
class GOMidiRecorder;
class GOOrgan;
class GOProgressDialog;
class GOSetter;
class GOConfig;
class GOTemperament;
class GODocument;
class GOSoundEngine;
class GOSoundProvider;
class GOSoundRecorder;
class GOSoundSampler;
typedef struct _GOHashType GOHashType;

class GOOrganController : public GOEventDistributor,
                          public GOTimer,
                          public GOOrganModel,
                          public GOModificationListener {
  WX_DECLARE_STRING_HASH_MAP(bool, GOStringBoolMap);

private:
  GODocument *m_doc;
  wxString m_odf;
  wxString m_ArchiveID;
  wxString m_ArchivePath;
  wxString m_hash;
  GOFileStore m_FileStore;
  wxString m_CacheFilename;
  wxString m_SettingFilename;
  wxString m_ODFHash;
  bool m_Cacheable;
  GOSetter *m_setter;
  GODivisionalSetter *m_DivisionalSetter;
  GOAudioRecorder *m_AudioRecorder;
  GOMidiPlayer *m_MidiPlayer;
  GOMidiRecorder *m_MidiRecorder;
  int m_volume;
  wxString m_Temperament;

  bool m_b_customized;
  float m_CurrentPitch; // organ pitch
  bool m_OrganModified; // always m_IsOrganModified >= IsModelModified()
  GOModificationListener *m_OrganModificationListener;
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
  ptr_vector<GOElementCreator> m_elementcreators;
  GOStringBoolMap m_UsedSections;

  GOSoundEngine *m_soundengine;
  GOMidi *m_midi;
  std::vector<bool> m_MidiSamplesetMatch;
  int m_SampleSetId1, m_SampleSetId2;
  GOGUIMouseState m_MouseState;

  GOMemoryPool m_pool;
  GOBitmapCache m_bitmaps;
  GOConfig &m_config;
  GOCombinationDefinition m_GeneralTemplate;
  GOLabelControl m_PitchLabel;
  GOLabelControl m_TemperamentLabel;
  GOMainWindowData m_MainWindowData;

  // if modified changes m_IsOrganModified then make a side effect
  void SetOrganModified(bool modified);
  // Called when the IsModelModified changed
  // if modified then sets m_IsOrganModified
  void OnIsModifiedChanged(bool modified);

  void ReadOrganFile(GOConfigReader &cfg);
  GOHashType GenerateCacheHash();
  wxString GenerateSettingFileName();
  wxString GenerateCacheFileName();
  void SetTemperament(const GOTemperament &temperament);
  void PreconfigRecorder();

  wxString GetOrganHash();

public:
  GOOrganController(GODocument *doc, GOConfig &settings);
  ~GOOrganController();

  // Returns organ modification flag
  bool IsOrganModified() const { return m_OrganModified; }
  void SetOrganModificationListener(GOModificationListener *listener) {
    m_OrganModificationListener = listener;
  }
  // Sets the organ modification flag
  void SetOrganModified() { SetOrganModified(true); }
  // Clears the organ modification flag
  void ResetOrganModified();

  const GOFileStore &GetFileStore() const { return m_FileStore; }

  /**
   * Set the organ directory without providing any odf.
   * Called only from GOPerfTest.
   * @param dir the directory for loading objects from
   */
  void InitOrganDirectory(const wxString &dir) {
    m_FileStore.SetDirectory(dir);
  }

  wxString Load(
    GOProgressDialog *dlg,
    const GOOrgan &organ,
    const wxString &cmb = wxEmptyString);
  void LoadCombination(const wxString &cmb);
  bool Save();
  bool Export(const wxString &cmb);
  bool CachePresent();
  bool IsCacheable();
  bool UpdateCache(GOProgressDialog *dlg, bool compress);
  void DeleteCache();
  void DeleteSettings();
  void Abort();
  void PreparePlayback(
    GOSoundEngine *engine, GOMidi *midi, GOSoundRecorder *recorder);
  void PrepareRecording();
  void Update();
  void Reset();
  void ProcessMidi(const GOMidiEvent &event);
  void AllNotesOff();
  GODocument *GetDocument();

  /* Access to internal ODF objects */
  GOSetter *GetSetter();
  GOGUIPanel *GetPanel(unsigned index);
  unsigned GetPanelCount();
  void AddPanel(GOGUIPanel *panel);
  GOMemoryPool &GetMemoryPool();
  GOConfig &GetSettings();
  GOBitmapCache &GetBitmapCache();
  void SetTemperament(wxString name);
  wxString GetTemperament();
  void MarkSectionInUse(wxString name);

  int GetRecorderElementID(wxString name);
  GOCombinationDefinition &GetGeneralTemplate();
  GOLabelControl *GetPitchLabel();
  GOLabelControl *GetTemperamentLabel();
  GOMainWindowData *GetMainWindowData();

  void LoadMIDIFile(const wxString &filename);

  /* ODF general properties */
  bool DivisionalsStoreIntermanualCouplers();
  bool DivisionalsStoreIntramanualCouplers();
  bool DivisionalsStoreTremulants();
  bool CombinationsStoreNonDisplayedDrawstops();
  bool GeneralsStoreDivisionalCouplers();

  void SetVolume(int volume);
  int GetVolume();

  unsigned GetReleaseTail() {
    return GetRootPipeConfigNode().GetEffectiveReleaseTail();
  }
  void SetReleaseTail(unsigned releaseTail) {
    GetRootPipeConfigNode().GetPipeConfig().SetReleaseTail(releaseTail);
  }

  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel = false);
  GOLabelControl *GetLabel(const wxString &name, bool is_panel = false);
  GOButtonControl *GetButtonControl(
    const wxString &name, bool is_panel = false);

  /* TODO: can somebody figure out what this thing is */
  bool IsCustomized();

  /* Filename of the organ definition used to load */
  const wxString GetODFFilename();
  const wxString GetOrganPathInfo();
  GOOrgan GetOrganInfo();
  const wxString GetSettingFilename();
  const wxString GetCacheFilename();

  /* Organ and Building general information */
  const wxString &GetChurchName();
  const wxString &GetChurchAddress();
  const wxString &GetOrganBuilder();
  const wxString &GetOrganBuildDate();
  const wxString &GetOrganComments();
  const wxString &GetRecordingDetails();
  const wxString &GetInfoFilename();

  GOSoundSampler *StartSample(
    const GOSoundProvider *pipe,
    int sampler_group_id,
    unsigned audio_group,
    unsigned velocity,
    unsigned delay,
    uint64_t last_stop);
  uint64_t StopSample(const GOSoundProvider *pipe, GOSoundSampler *handle);
  void SwitchSample(const GOSoundProvider *pipe, GOSoundSampler *handle);
  void UpdateVelocity(
    const GOSoundProvider *pipe, GOSoundSampler *handle, unsigned velocity);

  void SendMidiMessage(GOMidiEvent &e);
  void SendMidiRecorderMessage(GOMidiEvent &e);
  GOMidi *GetMidi();

  GOGUIMouseState &GetMouseState() { return m_MouseState; }
};

#endif
