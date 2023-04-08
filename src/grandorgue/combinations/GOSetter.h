/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTER_H
#define GOSETTER_H

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

  unsigned m_pos;
  unsigned m_bank;
  unsigned m_crescendopos;
  unsigned m_crescendobank;
  ptr_vector<GOGeneralCombination> m_framegeneral;
  ptr_vector<GOGeneralCombination> m_general;
  ptr_vector<GOGeneralCombination> m_crescendo;
  std::vector<GOCombination::ExtraElementsSet> m_CrescendoExtraSets;
  bool m_CrescendoOverrideMode[N_CRESCENDOS];
  GOLabelControl m_PosDisplay;
  GOLabelControl m_BankDisplay;
  GOLabelControl m_CrescendoDisplay;
  GOLabelControl m_TransposeDisplay;
  GOLabelControl m_NameDisplay;
  GOEnclosure m_swell;
  GOCombination::SetterType m_SetterType;

  void SetSetterType(GOCombination::SetterType type);
  void SetCrescendoType(unsigned no);
  void Crescendo(int pos, bool force = false);

  static const struct ButtonDefinitionEntry m_element_types[];
  const struct ButtonDefinitionEntry *GetButtonDefinitionList();

  void ButtonStateChanged(int id);

  void ControlChanged(void *control);

  void PreparePlayback();

public:
  GOSetter(GOOrganController *organController);
  virtual ~GOSetter();

  /**
   * Called when after combination is pushed
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
