/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTER_H
#define GOSETTER_H

#include <wx/arrstr.h>

#include "ptrvector.h"

#include "control/GOControlChangedHandler.h"
#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"
#include "model/GOCombination.h"
#include "model/GOEnclosure.h"
#include "sound/GOSoundStateHandler.h"
#include "yaml/GOSaveableToYaml.h"

#define N_CRESCENDOS 4

class GOGeneralCombination;

class GOSetter : private GOSoundStateHandler,
                 private GOControlChangedHandler,
                 public GOElementCreator,
                 public GOSaveableObject,
                 public GOSaveableToYaml {
private:
  GOOrganController *m_OrganController;

  // working with combination files
  wxString m_CmbFilesDir;
  wxArrayString m_CmbFileList;
  bool m_IsCmbFileListPopulated;
  // current loaded cmb file name (with dir and extension)
  wxString m_CmbFileLastLoaded;
  // has the current combinations been changed after last loaded or saved
  // used for lighting the save button
  bool m_IsCmbChanged;
  // current displayed cmb file name (with dir and extension)
  wxString m_CmbFileDisplayed;
  int m_CmbFilePos; // current displayed position or -1 or the list is not
                    // populated yet or no file are loaded

  unsigned m_pos;
  unsigned m_bank;
  unsigned m_crescendopos;
  unsigned m_crescendobank;
  ptr_vector<GOGeneralCombination> m_framegeneral;
  ptr_vector<GOGeneralCombination> m_general;
  ptr_vector<GOGeneralCombination> m_crescendo;
  std::vector<GOCombination::ExtraElementsSet> m_CrescendoExtraSets;
  bool m_CrescendoOverrideMode[N_CRESCENDOS];
  GOLabelControl m_CurrFileDisplay;
  GOLabelControl m_PosDisplay;
  GOLabelControl m_BankDisplay;
  GOLabelControl m_CrescendoDisplay;
  GOLabelControl m_TransposeDisplay;
  GOLabelControl m_NameDisplay;
  GOEnclosure m_swell;
  GOCombination::SetterType m_SetterType;

  // Show the combination file name
  void DisplayCmbFile(const wxString &fileName);
  /** Find yamlFile among the combination file list. Returns the
   * file index or to -1 if the file is not found
   */
  int FindCmbFilePosFor(const wxString &yamlFile);
  // Display the prev/next cmb file
  void MoveToCmbFile(int offset);

  void SetSetterType(GOCombination::SetterType type);
  void SetCrescendoType(unsigned no);
  void Crescendo(int pos, bool force = false);

  static const struct ButtonDefinitionEntry m_element_types[];
  const struct ButtonDefinitionEntry *GetButtonDefinitionList();

  void ButtonStateChanged(int id, bool newState) override;

  void ControlChanged(void *control);

  void PreparePlayback();

public:
  static const wxString KEY_REFRESH_FILES;
  static const wxString KEY_PREV_FILE;
  static const wxString KEY_CURR_FILE_NAME;
  static const wxString KEY_NEXT_FILE;
  static const wxString KEY_LOAD_FILE;
  static const wxString KEY_SAVE_FILE;
  static const wxString KEY_SAVE_SETTINGS;

  static const wxString GROUP_REFRESH_FILES;
  static const wxString GROUP_PREV_FILE;
  static const wxString GROUP_CURR_FILE_NAME;
  static const wxString GROUP_NEXT_FILE;
  static const wxString GROUP_LOAD_FILE;
  static const wxString GROUP_SAVE_FILE;
  static const wxString GROUP_SAVE_SETTINGS;

  GOSetter(GOOrganController *organController);
  virtual ~GOSetter();

  bool IsCmbModified() const { return m_IsCmbChanged; }

  /**
   * Called after at least one combination is changed
   * Temporary it calls mOrganController->SetModified()
   */
  void NotifyCmbChanged();
  /**
   * Called after a combination is pushed
   * When Set is active then marks the cpmbinations as modified
   * Temporary it calls mOrganController->SetModified()
   */
  void NotifyCmbPushed(bool isChanged = true);

  /**
   * Save all combinations to yaml as a map
   * @param outYaml - a yaml emitter to save combinations as a map
   */
  void ToYaml(YAML::Node &yamlNode) const override;
  void FromYaml(const YAML::Node &yamlNode) override;
  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel);

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

  bool StoreInvisibleObjects();
  bool IsSetterActive();
  void ToggleSetter();
  void SetterActive(bool on);
  GOCombination::SetterType GetSetterType() const { return m_SetterType; }

  /*
   * If current crescendo is in override mode then returns nullptr
   * If current crescendo is in add mode then fills elementSet and returns a
   * pointer to it
   */
  const GOCombination::ExtraElementsSet *GetCrescendoAddSet(
    GOCombination::ExtraElementsSet &elementSet);

  void Next();
  void Prev();
  void Push();
  unsigned GetPosition();
  void UpdatePosition(int pos);
  void SetPosition(int pos, bool push = true);
  void ResetDisplay();
  void SetTranspose(int value);
  void UpdateTranspose();
  void UpdateModified(bool modified);
};

#endif
