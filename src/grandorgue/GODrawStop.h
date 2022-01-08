/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODRAWSTOP_H
#define GODRAWSTOP_H

#include <wx/string.h>

#include "GOButton.h"
#include "GOCombinationElement.h"

class GODrawstop : public GOButton, public GOCombinationElement {
 public:
  typedef enum {
    FUNCTION_INPUT,
    FUNCTION_AND,
    FUNCTION_OR,
    FUNCTION_NOT,
    FUNCTION_XOR,
    FUNCTION_NAND,
    FUNCTION_NOR
  } GOFunctionType;

 private:
  static const struct IniFileEnumEntry m_function_types[];
  GOFunctionType m_Type;
  int m_GCState;
  bool m_ActiveState;
  bool m_CombinationState;
  std::vector<GODrawstop *> m_ControlledDrawstops;
  std::vector<GODrawstop *> m_ControllingDrawstops;

 protected:
  bool m_StoreDivisional;
  bool m_StoreGeneral;

  void SetState(bool on);
  virtual void ChangeState(bool on) = 0;
  virtual void SetupCombinationState() = 0;

  void Save(GOConfigWriter &cfg);

  void PreparePlayback();

 public:
  GODrawstop(GODefinitionFile *organfile);
  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group);
  void RegisterControlled(GODrawstop *sw);
  virtual void Set(bool on);
  virtual void Update();
  void Reset();
  void SetCombination(bool on);

  bool IsActive() const;
  bool GetCombinationState() const;
  bool GetStoreDivisional() const;
  bool GetStoreGeneral() const;
};

#endif
