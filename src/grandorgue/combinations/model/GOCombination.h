/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATION_H
#define GOCOMBINATION_H

#include <unordered_set>
#include <vector>

#include "yaml/GOSaveableToYaml.h"

class GOCombinationDefinition;
class GOOrganController;

class GOCombination : public GOSaveableToYaml {
public:
  enum SetterType { SETTER_REGULAR, SETTER_SCOPE, SETTER_SCOPED };
  using ExtraElementsSet = std::unordered_set<unsigned>;

private:
  GOOrganController *m_OrganFile;

protected:
  GOCombinationDefinition &m_Template;
  std::vector<int> m_State;
  bool m_Protected;

  void UpdateState();
  virtual bool PushLocal(ExtraElementsSet const *extraSet = nullptr);

public:
  GOCombination(
    GOCombinationDefinition &combination_template,
    GOOrganController *organController);
  virtual ~GOCombination();

  bool IsEmpty() const;
  int GetState(unsigned no);
  void SetState(unsigned no, int value);
  void GetExtraSetState(ExtraElementsSet &extraSet);
  void GetEnabledElements(GOCombination::ExtraElementsSet &enabledElements);

  void Copy(GOCombination *combination);
  void Clear();

  /**
   * Fills the combination from the current organ elements
   */
  bool FillWithCurrent(SetterType setterType, bool isToStoreInvisibleObjects);
  GOCombinationDefinition *GetTemplate();

  /**
   * If the combination is not empty, put it into the YAML map a a key value
   * pair
   *
   * key: combination
   *
   * @param container the yaml map to put the combination to
   * @param key the key for putting this combination with
   */
  void PutToYamlMap(YAML::Node &container, const char *key) const;
  void PutToYamlMap(YAML::Node &container, const wxString &key) const {
    PutToYamlMap(container, key.mbc_str().data());
  }
  static void putToYamlMap(
    YAML::Node &container, const wxString &key, const GOCombination *pCmb);
};

#endif
