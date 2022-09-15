/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTER_H
#define GOSETTER_H

#include "ptrvector.h"

#include "model/GOCombination.h"

#include "GOControlChangedHandler.h"
#include "GOElementCreator.h"
#include "GOEnclosure.h"
#include "GOLabel.h"
#include "GOPlaybackStateHandler.h"

#define N_CRESCENDOS 4

class GOGeneralCombination;

typedef enum { SETTER_REGULAR, SETTER_SCOPE, SETTER_SCOPED } SetterType;

class GOSetter : private GOPlaybackStateHandler,
                 private GOControlChangedHandler,
                 public GOElementCreator,
                 public GOSaveableObject {
private:
  GODefinitionFile *m_organfile;
  unsigned m_pos;
  unsigned m_bank;
  unsigned m_crescendopos;
  unsigned m_crescendobank;
  ptr_vector<GOGeneralCombination> m_framegeneral;
  ptr_vector<GOGeneralCombination> m_general;
  ptr_vector<GOGeneralCombination> m_crescendo;
  std::vector<GOCombination::ExtraElementsSet> m_CrescendoExtraSets;
  bool m_CrescendoOverrideMode[N_CRESCENDOS];
  GOLabel m_PosDisplay;
  GOLabel m_BankDisplay;
  GOLabel m_CrescendoDisplay;
  GOLabel m_TransposeDisplay;
  GOLabel m_NameDisplay;
  GOEnclosure m_swell;
  SetterType m_SetterType;

  void SetSetterType(SetterType type);
  void SetCrescendoType(unsigned no);
  void Crescendo(int pos, bool force = false);

  static const struct ElementListEntry m_element_types[];
  const struct ElementListEntry *GetButtonList();

  void ButtonChanged(int id);

  void ControlChanged(void *control);

  void AbortPlayback();
  void PreparePlayback();
  void StartPlayback();
  void PrepareRecording();

public:
  GOSetter(GODefinitionFile *organfile);
  virtual ~GOSetter();

  void Load(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabel *GetLabel(const wxString &name, bool is_panel);

  void Update();

  bool StoreInvisibleObjects();
  bool IsSetterActive();
  void ToggleSetter();
  void SetterActive(bool on);
  SetterType GetSetterType();

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
