/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODRAWSTOP_H
#define GODRAWSTOP_H

#include <wx/string.h>

#include "combinations/model/GOCombinationElement.h"
#include "control/GOButtonControl.h"

class GODrawstop : public GOButtonControl, public GOCombinationElement {
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
  /*
   * m_IsToStoreInDivisional and m_IsToStoreInGeneral control whether the
   * drawstop state should be saved in combinations by default.
   * When Full is enabled the state is saved regardless these variables.
   * The initial intention was to save the state of drawstops controlled by the
   * user and not to save the state that is calculated from the state of other
   * elements
   */
  bool m_IsToStoreInDivisional;
  bool m_IsToStoreInGeneral;

  virtual void SetupIsToStoreInCmb();

  void SetState(bool on);
  virtual void ChangeState(bool on) = 0;

  void Save(GOConfigWriter &cfg);

  void StartPlayback() override;

public:
  GODrawstop(GOOrganModel &organModel);

  bool IsToStoreInDivisional() const { return m_IsToStoreInDivisional; }
  bool IsToStoreInGeneral() const { return m_IsToStoreInGeneral; }
  bool IsActive() const { return m_ActiveState; }
  bool GetCombinationState() const override { return IsEngaged(); }

  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group);
  void RegisterControlled(GODrawstop *sw);
  virtual void Set(bool on);
  virtual void Update();
  void Reset();
  void SetCombination(bool on);
};

#endif
