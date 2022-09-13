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
class GODefinitionFile;

class GOCombination {
public:
  using ExtraElementsSet = std::unordered_set<unsigned>;

private:
  GODefinitionFile *m_OrganFile;

protected:
  GOCombinationDefinition &m_Template;
  std::vector<int> m_State;
  bool m_Protected;

  void UpdateState();
  virtual bool PushLocal(ExtraElementsSet const *extraSet = nullptr);

public:
  GOCombination(
    GOCombinationDefinition &combination_template, GODefinitionFile *organfile);
  virtual ~GOCombination();
  int GetState(unsigned no);
  void SetState(unsigned no, int value);
  void GetExtraSetState(ExtraElementsSet &extraSet);
  void Copy(GOCombination *combination);
  void Clear();
  GOCombinationDefinition *GetTemplate();
};

#endif
