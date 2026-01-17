/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTER_H
#define GOSETTER_H

#include <unordered_set>

#include <wx/arrstr.h>

#include "ptrvector.h"

#include "control/GOCombinationButtonSet.h"
#include "control/GOCombinationController.h"
#include "control/GOControlChangedHandler.h"
#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"
#include "model/GOCombination.h"
#include "model/GOEnclosure.h"
#include "model/GOGeneralCombination.h"
#include "model/GOSetterState.h"
#include "sound/GOSoundStateHandler.h"
#include "yaml/GOSaveableToYaml.h"

#define N_CRESCENDOS 4

class GODivisionalCombination;
class GOGeneralCombination;
class GOOrganController;

class GOSetter : private GOSoundStateHandler,
                 private GOCombinationButtonSet,
                 public GOCombinationController,
                 private GOControlChangedHandler,
                 public GOElementCreator,
                 public GOSaveableObject,
                 public GOSaveableToYaml {
public:
  enum {
    ID_SETTER_PREV = 0,
    ID_SETTER_NEXT,
    ID_SETTER_SET,
    ID_SETTER_M1,
    ID_SETTER_M10,
    ID_SETTER_M100,
    ID_SETTER_P1,
    ID_SETTER_P10,
    ID_SETTER_P100,
    ID_SETTER_CURRENT,
    ID_SETTER_HOME,
    ID_SETTER_GC,
    ID_SETTER_L0,
    ID_SETTER_L1,
    ID_SETTER_L2,
    ID_SETTER_L3,
    ID_SETTER_L4,
    ID_SETTER_L5,
    ID_SETTER_L6,
    ID_SETTER_L7,
    ID_SETTER_L8,
    ID_SETTER_L9,
    ID_SETTER_N,
    ID_SETTER_REGULAR,
    ID_SETTER_SCOPE,
    ID_SETTER_SCOPED,
    ID_SETTER_FULL,
    ID_SETTER_DELETE,
    ID_SETTER_INSERT,

    ID_SETTER_REFRESH_FILES,
    ID_SETTER_PREV_FILE,
    ID_SETTER_NEXT_FILE,
    ID_SETTER_LOAD_FILE,
    ID_SETTER_SAVE_FILE,
    ID_SETTER_SAVE_SETTINGS,

    ID_SETTER_GENERAL00,
    ID_SETTER_GENERAL01,
    ID_SETTER_GENERAL02,
    ID_SETTER_GENERAL03,
    ID_SETTER_GENERAL04,
    ID_SETTER_GENERAL05,
    ID_SETTER_GENERAL06,
    ID_SETTER_GENERAL07,
    ID_SETTER_GENERAL08,
    ID_SETTER_GENERAL09,
    ID_SETTER_GENERAL10,
    ID_SETTER_GENERAL11,
    ID_SETTER_GENERAL12,
    ID_SETTER_GENERAL13,
    ID_SETTER_GENERAL14,
    ID_SETTER_GENERAL15,
    ID_SETTER_GENERAL16,
    ID_SETTER_GENERAL17,
    ID_SETTER_GENERAL18,
    ID_SETTER_GENERAL19,
    ID_SETTER_GENERAL20,
    ID_SETTER_GENERAL21,
    ID_SETTER_GENERAL22,
    ID_SETTER_GENERAL23,
    ID_SETTER_GENERAL24,
    ID_SETTER_GENERAL25,
    ID_SETTER_GENERAL26,
    ID_SETTER_GENERAL27,
    ID_SETTER_GENERAL28,
    ID_SETTER_GENERAL29,
    ID_SETTER_GENERAL30,
    ID_SETTER_GENERAL31,
    ID_SETTER_GENERAL32,
    ID_SETTER_GENERAL33,
    ID_SETTER_GENERAL34,
    ID_SETTER_GENERAL35,
    ID_SETTER_GENERAL36,
    ID_SETTER_GENERAL37,
    ID_SETTER_GENERAL38,
    ID_SETTER_GENERAL39,
    ID_SETTER_GENERAL40,
    ID_SETTER_GENERAL41,
    ID_SETTER_GENERAL42,
    ID_SETTER_GENERAL43,
    ID_SETTER_GENERAL44,
    ID_SETTER_GENERAL45,
    ID_SETTER_GENERAL46,
    ID_SETTER_GENERAL47,
    ID_SETTER_GENERAL48,
    ID_SETTER_GENERAL49,
    ID_SETTER_GENERAL_PREV,
    ID_SETTER_GENERAL_NEXT,

    ID_SETTER_CRESCENDO_PREV,
    ID_SETTER_CRESCENDO_CURRENT,
    ID_SETTER_CRESCENDO_NEXT,
    ID_SETTER_CRESCENDO_A,
    ID_SETTER_CRESCENDO_B,
    ID_SETTER_CRESCENDO_C,
    ID_SETTER_CRESCENDO_D,
    ID_SETTER_CRESCENDO_OVERRIDE,

    ID_SETTER_TEMPERAMENT_PREV,
    ID_SETTER_TEMPERAMENT_NEXT,
    ID_SETTER_PITCH_M1,
    ID_SETTER_PITCH_M10,
    ID_SETTER_PITCH_M100,
    ID_SETTER_PITCH_P1,
    ID_SETTER_PITCH_P10,
    ID_SETTER_PITCH_P100,
    ID_SETTER_TRANSPOSE_DOWN,
    ID_SETTER_TRANSPOSE_UP,

    ID_SETTER_ON,
    ID_SETTER_AUDIO_PANIC,
    ID_SETTER_FILE_EXIT
  };
  // indexed by the enum
  static const ButtonDefinitionEntry *const P_BUTTON_DEFS;

private:
  GOOrganController *m_OrganController;

  // working with combination files
  wxString m_CmbFilesDir;
  wxArrayString m_CmbFileList;
  bool m_IsCmbFileListPopulated;
  // current loaded cmb file name (with dir and extension)
  wxString m_CmbFileLastLoaded;
  // current displayed cmb file name (with dir and extension)
  wxString m_CmbFileDisplayed;
  GOSetterState m_state;
  int m_CmbFilePos; // current displayed position or -1 or the list is not
                    // populated yet or no file are loaded

  unsigned m_pos;
  // -1 - not in the numeric mode. 0, 1, 2, 3 - the number of digits entered
  int m_NumericModeDigitsEntered;
  // The number already entered in the numeric mode
  unsigned m_NumericModeAccomulated;
  unsigned m_bank;
  unsigned m_crescendopos;
  unsigned m_crescendobank;
  ptr_vector<GOGeneralCombination> m_framegeneral;
  ptr_vector<GOGeneralCombination> m_general;
  ptr_vector<GOGeneralCombination> m_crescendo;
  bool m_CrescendoOverrideMode[N_CRESCENDOS];
  GOLabelControl m_CurrFileDisplay;
  GOLabelControl m_PosDisplay;
  GOLabelControl m_BankDisplay;
  GOLabelControl m_CrescendoDisplay;
  GOLabelControl m_TransposeDisplay;
  GOLabelControl m_NameDisplay;
  GOEnclosure m_CrescendoCtrl;

  // Show the combination file name
  void DisplayCmbFile(const wxString &fileName);
  /** Find yamlFile among the combination file list. Returns the
   * file index or to -1 if the file is not found
   */
  int FindCmbFilePosFor(const wxString &yamlFile);
  // Display the prev/next cmb file
  void MoveToCmbFile(int offset);

  void SetSetterType(GOSetterState::SetterType type);
  void SetCrescendoType(unsigned no);
  bool IsCurrentCrescendoOverride() const {
    return m_CrescendoOverrideMode[m_crescendobank];
  }
  wxString GetCrescendoCmbStateName(uint8_t crescendoIdx) const;
  void Crescendo(int pos, bool force = false);

  /**
   * Copy the sequencer combination
   * @param fromIdx - position of the source combination
   * @param toIdx - position of the destination combination
   * @param changedBefore - has the combination been changed before. If yes then
   *   do not check more for changing
   * @return if any of two combinations is changed or changedBefore
   */
  bool CopyFrameGenerals(unsigned fromIdx, unsigned toIdx, bool changedBefore);
  
  // Display entered number on m_PosDisplay in the N__ format
  void DisplayNumericPos();
  
  // Display the current sequencer position on m_PosDisplay in the 00N format
  void DisplayPos();

  void ButtonStateChanged(int id, bool newState) override;

  void ControlChanged(GOControl *control) override;

  void PreparePlayback() override;

  /**
   * Called after at least one combination is changed
   * Temporary it calls mOrganController->SetModified()
   */
  void NotifyCmbChanged();
  /**
   * Called after a combination is pushed
   * When Set is active then marks the combinations as modified
   * Temporary it calls mOrganController->SetModified()
   * @param isChanged is the combination actually changed
   * @param isForceSet is the combination memory changed even the Set button
   *   is not engaged (for example, Ins or Del are pushed)
   */
  void NotifyCmbPushed(bool isChanged = true, bool isForceSet = false);

  /**
   * Update all setter combination buttons light.
   * If the button
   * @param buttonToLight
   * @param manualIndexOnlyFor
   */
  void UpdateAllButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor) override;

  /**
   * Update buttons light in all combination button set.
   * If the button
   * @param buttonToLight
   * @param manualIndexOnlyFor
   */
  void UpdateAllSetsButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor);

public:
  static const wxString KEY_REFRESH_FILES;
  static const wxString KEY_PREV_FILE;
  static const wxString KEY_CURR_FILE_NAME;
  static const wxString KEY_NEXT_FILE;
  static const wxString KEY_LOAD_FILE;
  static const wxString KEY_SAVE_FILE;
  static const wxString KEY_SAVE_SETTINGS;

  static const wxString KEY_ON_STATE;

  static const wxString GROUP_REFRESH_FILES;
  static const wxString GROUP_PREV_FILE;
  static const wxString GROUP_CURR_FILE_NAME;
  static const wxString GROUP_NEXT_FILE;
  static const wxString GROUP_LOAD_FILE;
  static const wxString GROUP_SAVE_FILE;
  static const wxString GROUP_SAVE_SETTINGS;

  GOSetter(GOOrganController *organController);
  virtual ~GOSetter();

  const GOSetterState &GetState() const { return m_state; }
  bool IsCmbModified() const { return m_state.m_IsModified; }

  /**
   * Save all combinations to yaml as a map
   * @param outYaml - a yaml emitter to save combinations as a map
   */
  void ToYaml(YAML::Node &yamlNode) const override;
  void FromYaml(const YAML::Node &yamlNode) override;
  void Load(GOConfigReader &cfg) override;
  void Save(GOConfigWriter &cfg) override;
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) override;
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel) override;

  /**
   * Called from GOOrganController after loading the initial preset and after
   * loading a yaml combination file.
   * GOSetter takes yamlDir for navigation across the combination
   * files and takes yamlFile for displaying it.
   * @param yamlDir full path to the directory where the yaml combination files
   *   are stored for this organ or where the last combination file has been
   *   loaded from
   * @param yamlFile full file name of the last loaded file name with the
   *   directory. If a preset or a cmb-combination is loaded, then
   *   yamlFile is empty
   */
  void OnCombinationsLoaded(const wxString &yamlDir, const wxString &yamlFile);

  /**
   * Called from GOOrganController after saving the current combinations to a
   * yamlFile.
   * If the yamlFile is the same as m_CmbFileLastLoaded then reset
   * m_IsCmbChanged. Else, if the file in the same directory, reset
   * m_IsCmbFileListPopulated
   * @param yamlFile the full file name where the combinations were saved
   */
  void OnCombinationsSaved(const wxString &yamlFile);

  void Update();

  void ToggleSetter();
  void SetterActive(bool on);

  /*
   * Activate cmb
   * If the crescendo in add mode then not to disable stops that are present in
   * extraSet
   * If isFromCrescendo and it is in add mode then then does not depress other
   * buttons
   * @param cmb the cmb to activate or to set
   * @param pButton the button to light on
   * return if anything is changed
   */
  void PushGeneral(
    GOGeneralCombination &cmb, GOButtonControl *pButtonToLight) override;
  void PushDivisional(
    GODivisionalCombination &cmb,
    unsigned startManual,
    unsigned cmbManual,
    GOButtonControl *pButtonToLight) override;

  void Next();
  void Prev();
  void Push();
  unsigned GetPosition();
  void UpdatePosition(int pos);
  void SetPosition(int pos, bool push = true);

  /**
   * Switch the light of all combination buttons off
   */
  void ResetCmbButtons() { UpdateAllSetsButtonsLight(nullptr, -1); }
  void SetTranspose(int value);
  void UpdateTranspose();
  void UpdateModified(bool modified);
};

#endif
