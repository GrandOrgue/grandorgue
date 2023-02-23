/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
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

#define N_CRESCENDOS 4

class GOGeneralCombination;

typedef enum { SETTER_REGULAR, SETTER_SCOPE, SETTER_SCOPED } SetterType;

class GOSetter : private GOSoundStateHandler,
                 private GOControlChangedHandler,
                 public GOElementCreator,
                 public GOSaveableObject {
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
  SetterType m_SetterType;

  void SetSetterType(SetterType type);
  void SetCrescendoType(unsigned no);
  void Crescendo(int pos, bool force = false);

  static const struct ButtonDefinitionEntry m_element_types[];
  const struct ButtonDefinitionEntry *GetButtonDefinitionList();

  void ButtonStateChanged(int id);

  void ControlChanged(void *control);

  void PreparePlayback();
  void StartPlayback();

public:
  GOSetter(GOOrganController *organController);
  virtual ~GOSetter();

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel);

  void Update();

  bool StoreInvisibleObjects();
  bool IsSetterActive();
  void ToggleSetter();
  void SetterActive(bool on);
  SetterType GetSetterType();

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
