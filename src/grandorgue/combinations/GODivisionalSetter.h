/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIVISIONALSETTER_H
#define GODIVISIONALSETTER_H

#include <cstdint>
#include <map>
#include <unordered_map>

#include <wx/hashset.h>

#include "ptrvector.h"

#include "control/GOCombinationButtonSet.h"
#include "control/GOElementCreator.h"
#include "yaml/GOSaveableToYaml.h"

#include "GOSaveableObject.h"

class GOOrganController;
class GODivisionalCombination;
class GOLabelControl;
class GOSetterState;

class GODivisionalSetter : public GOElementCreator,
                           private GOCombinationButtonSet,
                           GOSaveableObject,
                           public GOSaveableToYaml {
private:
  // Maps combination numbers to defined combinations
  // The first bank (A) has numbers from 0 to 9, the second one - from 10 to 19
  // If some number is absent then the combination is not defined yet
  using DivisionalMap = std::map<unsigned, GODivisionalCombination *>;

  GOOrganController *m_OrganController;
  const GOSetterState &r_SetterState;

  // the setter starts manuals from 0 but m_OrganController may start from
  // m_FirstManualIndex
  unsigned m_FirstManualIndex;
  // the maximal manual index in odf + 1
  unsigned m_OdfManualCount;
  // the actual number of manuals. m_OdfManualCount = m_NManuals +
  // m_FirstManualIndex
  unsigned m_NManuals;
  // Button definitions for creating button controls
  ButtonDefinitionEntry *m_ButtonDefinitions;
  // Labels displaying the current bank for each manual
  ptr_vector<GOLabelControl> m_BankLabels;
  // The same labels for searching by Setter element name
  std::
    unordered_map<const wxString, GOLabelControl *, wxStringHash, wxStringEqual>
      m_BankLabelsByName;
  // current bank numbers for each manual. 0 - A, 1 - B, etc
  std::vector<uint8_t> m_manualBanks;
  // All combinations for all nanuals
  std::vector<DivisionalMap> m_DivisionalMaps;

  // update the current bank on the m_BankLabels[manualN]
  void UpdateBankDisplay(unsigned manualN, uint8_t bankN);

  /**
   * Switches the current bank of the manual.
   * @param manualN manual number in the setter
   * @param setNewBank a function with (unsigned &currBank) parameter. It may
   *   change currBank
   */
  template <typename F> void SwitchBank(unsigned manualN, const F &setNewBank);

  // delete all combinations from m_DivisionalMaps
  void ClearCombinations();

  /**
   * Update all divisional buttons light.
   * @param buttonToLight - the button that should be lighted on. All other
   *   divisionals are lighted off
   * @param manualIndexOnlyFor - if >= 0 change lighting of this manual only
   */
  void UpdateAllButtonsLight(
    GOButtonControl *buttonToLight, int manualIndexOnlyFor) override;

protected:
  // called from GOElementCreator::CreateButtons()
  const struct GOElementCreator::ButtonDefinitionEntry *
  GetButtonDefinitionList() override;

  // called on pressing any button. id is mapped to manualN and divisionalN
  void ButtonStateChanged(int id, bool newState) override;

public:
  // calculates the setter element name for a divisional button
  static wxString GetDivisionalButtonName(
    unsigned manualIndex, unsigned divisionalIndex);
  // calculates the setter element name for the nank label
  static wxString GetDivisionalBankLabelName(unsigned manualIndex);
  // calculates the setter element name for the prev-bank button
  static wxString GetDivisionalBankPrevLabelName(unsigned manualIndex);
  // calculates the setter element name for the next-bank button
  static wxString GetDivisionalBankNextLabelName(unsigned manualIndex);

  GODivisionalSetter(
    GOOrganController *organController, const GOSetterState &setterState);
  virtual ~GODivisionalSetter();

  // saves all combinations to the preset file
  virtual void Save(GOConfigWriter &cfg) override;
  // loads combinations from the preset file
  virtual void LoadCombination(GOConfigReader &cfg) override;
  // loads definitions of all buttons
  virtual void Load(GOConfigReader &cfg) override;

  /**
   * Save the combination to the YAML object
   */
  void ToYaml(YAML::Node &yamlNode) const override;

  /**
   * Loads the object from the saved object in Yaml
   * @param yamlNode - a node with the saved object
   */
  void FromYaml(const YAML::Node &yamlNode) override;

  // it is not used but it is required to be as it is declared in
  // GOElementCreator
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) override {
    return nullptr;
  }

  // returns a label by the setter element name. Only current bank labels
  GOLabelControl *GetLabelControl(
    const wxString &name, bool is_panel) override {
    return m_BankLabelsByName[name];
  }

  // Activate the previous bank for the manual if it exists.
  // manualN starts with 0
  void SwitchBankToPrev(unsigned manualN);

  // Activate the next bank for the manual if it exists
  // manualN starts with 0
  void SwitchBankToNext(unsigned manualN);

  // Activates the combination for the manual as the button divisionalN is
  // pressed. Current bank is taken into account.
  // manualN, and divisionalN start with 0
  void SwitchDivisionalTo(unsigned manualN, unsigned divisionalN);
};

#endif /* GODIVISIONALSETTER_H */
