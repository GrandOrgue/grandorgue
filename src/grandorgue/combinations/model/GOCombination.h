/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATION_H
#define GOCOMBINATION_H

#include <unordered_set>
#include <vector>

class GOCombinationDefinition;
class GOOrganController;

class GOCombination {
public:
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
  int GetState(unsigned no);
  void SetState(unsigned no, int value);
  void GetExtraSetState(ExtraElementsSet &extraSet);
  void GetEnabledElements(GOCombination::ExtraElementsSet &enabledElements);

  void Copy(GOCombination *combination);
  void Clear();
  GOCombinationDefinition *GetTemplate();
};

#endif
