/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANCONTROLLER_H
#define GOORGANCONTROLLER_H

#include <vector>

#include <wx/filefn.h>
#include <wx/string.h>

#include "ptrvector.h"

#include "config/GOConfig.h"
#include "control/GOEventDistributor.h"
#include "control/GOLabelControl.h"
#include "gui/frames/GOMainWindowData.h"
#include "gui/panels/GOGUIMouseState.h"
#include "loader/GOFileStore.h"
#include "loader/GOLoadedOrganInfo.h"
#include "loader/GOProgressMonitor.h"
#include "model/GOOrganModel.h"
#include "modification/GOModificationProxy.h"

#include "sound/GOSoundOrganEngine.h"

#include "GOMemoryPool.h"
#include "GOOrgan.h"
#include "GOVirtualCouplerController.h"

class GOArchive;
class GOAudioRecorder;
class GOButtonControl;
class GOCache;
class GOConfigWriter;
class GODialogSizeSet;
class GODivisionalSetter;
class GOElementCreator;
class GOGuiImageCache;
class GOGuiOrgan;
class GOGUIPanel;
class GOMidiEvent;
class GOMidiPlayer;
class GOMidiRecorder;
class GOMidiSystem;
class GOSetter;
class GOSoundProvider;
class GOSoundRecorderTask;
class GOSoundSystem;
class GOTemperament;
class GOTimer;
typedef struct _GOHashType GOHashType;

class GOOrganController : public GOEventDistributor,
                          public GOOrganModel,
                          public GOModificationProxy {
  // Exercises LoadOrganCoreData()/LoadObjects()/SaveOrganCoreData()/
  // ClearObjects()/ClearOrganCoreData() directly, bypassing Load()'s
  // unconditional call to OnLoad() (which needs a real GUI display).
  friend class GOTestOrganController;

private:
  GOConfig &m_config;
  GOOrgan m_ConfiguredOrgan;
  GOLoadedOrganInfo m_LoadedOrganInfo;
  GOFileStore m_FileStore;
  bool m_Cacheable;
  bool m_IsOrganCoreDataLoaded;
  bool m_IsOrganGuiLoaded;
  bool m_IsObjectsLoaded;
  GOSetter *m_setter;
  GODivisionalSetter *m_DivisionalSetter;
  GOAudioRecorder *m_AudioRecorder;
  GOMidiPlayer *m_MidiPlayer;
  GOMidiRecorder *m_MidiRecorder;
  GOSizeKeeper m_StopWindowSizeKeeper;
  GOTimer *m_timer;
  GOButtonControl *p_OnStateButton;
  wxString m_Temperament;

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
  ptr_vector<GOElementCreator> m_elementcreators;

  GOMidiSystem *m_midi;
  std::vector<bool> m_MidiSamplesetMatch;
  int m_SampleSetId1, m_SampleSetId2;
  GOGUIMouseState m_MouseState;

  GOMemoryPool m_pool;
  GOSoundOrganEngine m_SoundEngine;
  GOGuiImageCache *mp_ImageCache;
  GOLabelControl m_PitchLabel;
  GOLabelControl m_TemperamentLabel;
  GOMainWindowData m_MainWindowData;

  // if modified changes m_IsOrganModified then make a side effect
  void SetOrganModified(bool modified);
  // Called when the IsModelModified changed
  // if modified then sets m_IsOrganModified
  void OnIsModifiedChanged(bool modified);

  /** Reads the non-GUI ODF/CMB data (church info, model, element creators,
   * combinations) into this controller. Sets m_IsOrganCoreDataLoaded. */
  void LoadOrganCoreData(GOConfigReader &cfg);
  /** Hook for a subclass to load GUI-only data, called after
   * LoadOrganCoreData() and before the ODF/CMB unused-key report. Empty by
   * default - a bare GOOrganController has no GUI. Overridden by
   * GOGuiOrgan::Controller, which calls LoadOrganGuiData() below (still
   * owned by this class) before building its own panel creators.
   * @param cfg the config reader for the ODF/CMB currently being loaded,
   *   the same one passed to LoadOrganCoreData() */
  virtual void OnLoad(GOConfigReader &cfg) {}
  /** Loads pipe/sample data from the cache or, failing that, from the
   * sample files in parallel worker threads. Sets m_IsObjectsLoaded. */
  void LoadObjects(GOProgressMonitor &monitor);
  /** Writes the non-GUI organ state (church info, volume, temperament,
   * saveable objects, virtual couplers) to cfg. */
  void SaveOrganCoreData(GOConfigWriter &cfg);
  /** Hook for a subclass to save GUI-only data, called after
   * SaveOrganCoreData() and before the file is written. Empty by default.
   * @param cfg the config writer Save() is assembling; core data has
   *   already been written to it by the time this runs */
  virtual void OnSave(GOConfigWriter &cfg);
  /** Undoes LoadObjects if it ran. Idempotent. */
  void ClearObjects();
  /** Hook for a subclass to clear GUI-only data, called after
   * ClearObjects() and before ClearOrganCoreData() (GUI data may reference
   * core model objects that ClearOrganCoreData() frees). Empty by default.
   * Overridden by GOGuiOrgan::Controller, which calls ClearOrganGuiData()
   * below (still owned by this class) to clear its own panel creators. */
  virtual void OnClear() {}
  /** Undoes LoadOrganCoreData if it ran. Idempotent. */
  void ClearOrganCoreData();
  GOHashType GenerateCacheHash();
  void SetTemperament(const GOTemperament &temperament);
  void PreconfigRecorder();

protected:
  /**
   * Loads the GUI-only data still owned by this class (panels, image
   * cache, main-window data, stop-window size) - everything OnLoad() used
   * to do except building/running the panel creators, which
   * GOGuiOrgan::Controller now owns and runs after this returns. Called
   * from GOGuiOrgan::Controller::OnLoad(). Temporary: GOGuiOrgan will take
   * over this data too once the ownership move is complete, at which point
   * this method goes away.
   * @param cfg the config reader for the ODF/CMB currently being loaded
   */
  void LoadOrganGuiData(GOConfigReader &cfg);
  /** Undoes LoadOrganGuiData(). Called from
   * GOGuiOrgan::Controller::OnClear(). Temporary, see LoadOrganGuiData(). */
  void ClearOrganGuiData();

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
    const GOOrgan &organ,
    const wxString &cmb,
    bool isGuiOnly,
    GOProgressMonitor &monitor);
  /** Undoes whatever Load() built (core data, GUI, cached objects), in
   * reverse order. Idempotent - safe to call any number of times.
   * Callers must call this explicitly before destroying the object (stack,
   * member, or heap): Clear() calls the virtual OnClear(), and a call made
   * from within ~GOOrganController() would only ever reach
   * GOOrganController::OnClear(), never a subclass override, since C++
   * virtual dispatch during base-class destruction is restricted to the
   * base class. The destructor only asserts that ClearObjects()/OnClear()
   * already ran; it still re-runs the non-virtual ClearOrganCoreData() on
   * its own as a safety net regardless of whether the caller called
   * Clear(). */
  void Clear();
  /**
   * Exports organ combinations in the yaml file
   * @param fileName - the path to the yaml file to export
   * @return an empty string if succeeded otherwise the error message
   */
  wxString ExportCombination(const wxString &fileName);
  void LoadCombination(const wxString &cmb);
  /**
   * Writes the organ's core and GUI data to a config file.
   * @param path the file to write to; empty (the default) means the path
   *   this organ was loaded from (m_LoadedOrganInfo.settingsFilePath) - a
   *   default member expression can't be used here since default arguments
   *   can't reference `this`, so the sentinel is resolved in the body.
   *   An explicit non-empty path is treated as an export to a copy and does
   *   not reset the modified flag.
   * @return true if the file was written successfully
   */
  bool Save(const wxString &path = wxEmptyString);
  bool CachePresent() const {
    return wxFileExists(m_LoadedOrganInfo.cacheFilePath);
  }
  bool IsCacheable() const { return m_Cacheable; }
  bool UpdateCache(bool compress, GOProgressMonitor &monitor);
  void DeleteCache();
  void DeleteSettings();
  void PrepareRecording();

  /** Returns true if the organ sound engine is currently running. */
  bool IsOrganStarted() const { return m_SoundEngine.IsWorking(); }

  /**
   * Starts the organ sound engine: builds audio tasks, connects to the sound
   * system, and begins MIDI and audio playback.
   */
  void StartOrgan(GOSoundSystem &soundSystem, GOMidiSystem &midi);

  /**
   * Stops the organ sound engine: aborts playback, disconnects from the sound
   * system, and tears down audio tasks.
   */
  void StopOrgan(GOSoundSystem &soundSystem);
  GOSoundOrganEngine &GetSoundEngine() { return m_SoundEngine; }
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
  GOGuiImageCache &GetImageCache() const { return *mp_ImageCache; }
  void SetTemperament(const wxString &name);
  const wxString &GetTemperament() const { return m_Temperament; }

  GOLabelControl *GetPitchLabel() { return &m_PitchLabel; }
  GOLabelControl *GetTemperamentLabel() { return &m_TemperamentLabel; }
  GOMainWindowData *GetMainWindowData() { return &m_MainWindowData; }

  /** @return the virtual coupler configuration for this organ, used by
   * GOGUICouplerPanel when building the coupler panel. */
  const GOVirtualCouplerController &GetVirtualCouplers() const {
    return m_VirtualCouplers;
  }

  /**
   * Loads a MIDI file for playback via the MIDI player.
   * @param filename the MIDI file to load
   * @param chooseMapping see GOMidiPlayer::LoadFile()
   */
  void LoadMIDIFile(
    const wxString &filename,
    const GOConfig::MidiChannelMappingChooser &chooseMapping);

  int GetVolume() const { return m_SoundEngine.GetVolume(); }
  /** Sets the master volume and forwards it to the sound engine. */
  void SetVolume(int volume) { m_SoundEngine.SetVolume(volume); }

  /** Returns true if the sound engine is running. */
  bool IsStarted() const { return m_SoundEngine.IsWorking(); }

  /** Sets the polyphony hard limit in the sound engine. */
  void SetHardPolyphony(unsigned polyphony) {
    m_SoundEngine.SetHardPolyphony(polyphony);
  }

  /**
   * Returns meter info from the sound engine.
   */
  std::vector<float> GetMeterInfo() { return m_SoundEngine.GetMeterInfo(); }

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
  bool IsCustomized() const { return m_LoadedOrganInfo.isCustomized; }

  /* Filename of the organ definition used to load */
  const wxString &GetODFFilename() const {
    return m_ConfiguredOrgan.GetODFPath();
  }
  const wxString GetOrganPathInfo();
  GOOrgan GetOrganInfo();
  const wxString &GetSettingFilename() const {
    return m_LoadedOrganInfo.settingsFilePath;
  }
  const wxString &GetCacheFilename() const {
    return m_LoadedOrganInfo.cacheFilePath;
  }
  wxString GetCombinationsDir() const;

  /* Organ and Building general information */
  const wxString &GetChurchAddress() const { return m_ChurchAddress; }
  const wxString &GetOrganBuilder() const { return m_OrganBuilder; }
  const wxString &GetOrganBuildDate() const { return m_OrganBuildDate; }
  const wxString &GetOrganComments() const { return m_OrganComments; }
  const wxString &GetRecordingDetails() const { return m_RecordingDetails; }
  const wxString &GetInfoFilename() const { return m_InfoFilename; }

  GOMidiSystem *GetMidi() { return m_midi; }

  GOGUIMouseState &GetMouseState() { return m_MouseState; }

  /**
   * Return the Timer Manager for Metronome, Midi recorder, ...
   */
  GOTimer *GetTimer() const { return m_timer; }
};

#endif
