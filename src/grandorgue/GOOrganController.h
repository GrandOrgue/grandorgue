/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANCONTROLLER_H
#define GOORGANCONTROLLER_H

#include <vector>

#include <wx/string.h>

#include "ptrvector.h"

#include "config/GOConfig.h"
#include "control/GOEventDistributor.h"
#include "control/GOLabelControl.h"
#include "gui/frames/GOMainWindowData.h"
#include "gui/panels/GOGUIMouseState.h"
#include "loader/GOFileStore.h"
#include "model/GOOrganModel.h"
#include "modification/GOModificationProxy.h"

#include "GOBitmapCache.h"
#include "GOMemoryPool.h"
#include "GOTimer.h"
#include "GOVirtualCouplerController.h"

class GOGUIPanel;
class GOGUIPanelCreator;
class GOGUICouplerPanel;
class GOArchive;
class GOAudioRecorder;
class GOButtonControl;
class GOCache;
class GODialogSizeSet;
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
typedef struct _GOHashType GOHashType;

class GOOrganController : public GOEventDistributor,
                          public GOOrganModel,
                          public GOModificationProxy {
private:
  GOConfig &m_config;
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
  GOSizeKeeper m_StopWindowSizeKeeper;
  GOTimer *m_timer;
  GOButtonControl *p_OnStateButton;
  int m_volume;
  wxString m_Temperament;

  bool m_b_customized;
  float m_CurrentPitch; // organ pitch
  bool m_OrganModified; // always m_IsOrganModified >= IsModelModified()

  wxString m_ChurchAddress;
  wxString m_OrganBuilder;
  wxString m_OrganBuildDate;
  wxString m_OrganComments;
  wxString m_RecordingDetails;
  wxString m_InfoFilename;

  GOVirtualCouplerController m_VirtualCouplers;

  ptr_vector<GOGUIPanel> m_panels;
  ptr_vector<GOGUIPanelCreator> m_panelcreators;
  ptr_vector<GOElementCreator> m_elementcreators;

  GOSoundEngine *m_soundengine;
  GOMidi *m_midi;
  std::vector<bool> m_MidiSamplesetMatch;
  int m_SampleSetId1, m_SampleSetId2;
  GOGUIMouseState m_MouseState;

  GOMemoryPool m_pool;
  GOBitmapCache *m_bitmaps;
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

  const wxString &GetOrganHash() const { return m_hash; }

public:
  GOOrganController(GOConfig &config, bool isAppInitialized = false);
  virtual ~GOOrganController();

  GOSizeKeeper &GetStopWindowSizeKeeper() { return m_StopWindowSizeKeeper; }

  // Returns organ modification flag
  bool IsOrganModified() const { return m_OrganModified; }
  // Sets the organ modification flag
  void SetOrganModified() { SetOrganModified(true); }
  // Clears the organ modification flag
  void ResetOrganModified();

  const GOFileStore &GetFileStore() const { return m_FileStore; }
  GODialogSizeSet &GetDialogSizeSet() { return m_config.m_DialogSizes; }

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
    const wxString &cmb,
    bool isGuiOnly);
  /**
   * Exports organ combinations in the yaml file
   * @param fileName - the path to the taml file to export
   * @return an empty string if successed otherwise the error message
   */
  wxString ExportCombination(const wxString &fileName);
  void LoadCombination(const wxString &cmb);
  bool Save();
  bool Export(const wxString &cmb);
  bool CachePresent() const { return wxFileExists(m_CacheFilename); }
  bool IsCacheable() const { return m_Cacheable; }
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
  // GODocument *GetDocument();

  /* Access to internal ODF objects */
  GOSetter *GetSetter() const { return m_setter; }
  GOGUIPanel *GetPanel(unsigned index) { return m_panels[index]; }
  unsigned GetPanelCount() const { return m_panels.size(); }
  void AddPanel(GOGUIPanel *panel) { m_panels.push_back(panel); }
  GOMemoryPool &GetMemoryPool() { return m_pool; }
  GOConfig &GetSettings() { return m_config; }
  GOBitmapCache &GetBitmapCache() const { return *m_bitmaps; }
  void SetTemperament(const wxString &name);
  const wxString &GetTemperament() const { return m_Temperament; }

  GOLabelControl *GetPitchLabel() { return &m_PitchLabel; }
  GOLabelControl *GetTemperamentLabel() { return &m_TemperamentLabel; }
  GOMainWindowData *GetMainWindowData() { return &m_MainWindowData; }

  void LoadMIDIFile(const wxString &filename);

  void SetVolume(int volume) { m_volume = volume; }
  int GetVolume() const { return m_volume; }

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
  bool IsCustomized() const { return m_b_customized; }

  /* Filename of the organ definition used to load */
  const wxString &GetODFFilename() const { return m_odf; }
  const wxString GetOrganPathInfo();
  GOOrgan GetOrganInfo();
  const wxString &GetSettingFilename() const { return m_SettingFilename; }
  const wxString &GetCacheFilename() const { return m_CacheFilename; }
  wxString GetCombinationsDir() const;

  /* Organ and Building general information */
  const wxString &GetChurchAddress() const { return m_ChurchAddress; }
  const wxString &GetOrganBuilder() const { return m_OrganBuilder; }
  const wxString &GetOrganBuildDate() const { return m_OrganBuildDate; }
  const wxString &GetOrganComments() const { return m_OrganComments; }
  const wxString &GetRecordingDetails() const { return m_RecordingDetails; }
  const wxString &GetInfoFilename() const { return m_InfoFilename; }

  GOMidi *GetMidi() { return m_midi; }

  GOGUIMouseState &GetMouseState() { return m_MouseState; }

  /**
   * Return the Timer Manager for Metronome, Midi recorder, ...
   */
  GOTimer *GetTimer() const { return m_timer; }
};

#endif
