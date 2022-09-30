/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALSETTER_H
#define GODIVISIONALSETTER_H

#include <map>
#include <unordered_map>

#include <wx/hashset.h>

#include "ptrvector.h"

#include "control/GOElementCreator.h"

#include "GOSaveableObject.h"

class GODefinitionFile;
class GODivisionalCombination;
class GOLabelControl;

class GODivisionalSetter : public GOElementCreator, GOSaveableObject {
private:
  using DivisionalMap = std::map<unsigned, GODivisionalCombination *>;

  GODefinitionFile *m_organfile;
  unsigned m_FirstManualIndex;
  unsigned m_OdfManualCount;
  unsigned m_NManuals;
  ButtonDefinitionEntry *m_ButtonDefinitions;
  ptr_vector<GOLabelControl> m_BankLabels;
  std::
    unordered_map<const wxString, GOLabelControl *, wxStringHash, wxStringEqual>
      m_BankLabelsByName;
  std::vector<unsigned> m_manualBanks;
  std::vector<DivisionalMap> m_DivisionalMaps;
  void UpdateBankDisplay(unsigned manualN);
  void ClearCombinations();

protected:
  const struct GOElementCreator::ButtonDefinitionEntry *
  GetButtonDefinitionList();

  void ButtonStateChanged(int id);

public:
  static wxString GetDivisionalButtonName(
    unsigned manualIndex, unsigned divisionalIndex);
  static wxString GetDivisionalBankLabelName(unsigned manualIndex);
  static wxString GetDivisionalBankPrevLabelName(unsigned manualIndex);
  static wxString GetDivisionalBankNextLabelName(unsigned manualIndex);

  GODivisionalSetter(GODefinitionFile *organfile);
  virtual ~GODivisionalSetter();

  virtual void Save(GOConfigWriter &cfg);
  virtual void LoadCombination(GOConfigReader &cfg);
  virtual void Load(GOConfigReader &cfg);

  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) {
    return nullptr;
  }

  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel) {
    return m_BankLabelsByName[name];
  }

  void SwitchDivisionalTo(unsigned manualN, unsigned divisionalN);

  void SwitchBankToPrev(unsigned manualN);

  void SwitchBankToNext(unsigned manualN);
};

#endif /* GODIVISIONALSETTER_H */
