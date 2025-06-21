/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODRAWSTOP_H
#define GODRAWSTOP_H

#include <unordered_map>

#include <wx/hashmap.h>
#include <wx/string.h>

#include "combinations/model/GOCombinationElement.h"
#include "control/GOButtonControl.h"

class GODrawstop : public GOButtonControl, virtual public GOCombinationElement {
public:
  enum GOFunctionType {
    FUNCTION_INPUT,
    FUNCTION_AND,
    FUNCTION_OR,
    FUNCTION_NOT,
    FUNCTION_XOR,
    FUNCTION_NAND,
    FUNCTION_NOR
  };

private:
  GOFunctionType m_Type;
  int m_GCState;
  std::unordered_map<wxString, bool, wxStringHash, wxStringEqual>
    m_InternalStates;
  std::vector<GODrawstop *> m_ControlledDrawstops;
  std::vector<GODrawstop *> m_ControllingDrawstops;

  bool IsControlledByUser() const override { return !IsReadOnly(); }

  // set the result state of the internal OR switch
  void SetResultState(bool on);
  // set one entry of the internal OR switch
  bool CalculateResultState(bool includeDefault) const;
  void SetInternalState(bool on, const wxString &stateName);
  // set the default entry of the internal OR switch
  void SetDrawStopState(bool on) { SetInternalState(on, wxEmptyString); }

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

  virtual void OnDrawstopStateChanged(bool on) = 0;

  void Save(GOConfigWriter &cfg) override;

  void StartPlayback() override;

public:
  GODrawstop(GOOrganModel &organModel, ObjectType objectType);

  /* + For tests only */
  GOFunctionType GetFunctionType() const { return m_Type; }
  void SetFunctionType(GOFunctionType newFunctionType);

  void ClearControllingDrawstops();
  void AddControllingDrawstop(
    GODrawstop *pDrawStop, unsigned switchN, const wxString &group);
  /* - For tests only */

  bool IsToStoreInDivisional() const { return m_IsToStoreInDivisional; }
  bool IsToStoreInGeneral() const { return m_IsToStoreInGeneral; }
  bool GetCombinationState() const override { return IsEngaged(); }

  void Init(
    GOConfigReader &cfg, const wxString &group, const wxString &name) override;

  void Load(GOConfigReader &cfg, const wxString &group) override;
  void RegisterControlled(GODrawstop *sw);
  void UnRegisterControlled(GODrawstop *sw);
  virtual void SetButtonState(bool on) override;
  virtual void Update();
  void Reset();
  void SetCombinationState(bool on, const wxString &stateName) override;
};

#endif
