/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOMBINATION_H
#define GOCOMBINATION_H

#include <vector>

class GOCombinationDefinition;
class GODefinitionFile;

class GOCombination {
 private:
  GODefinitionFile* m_OrganFile;

 protected:
  GOCombinationDefinition& m_Template;
  std::vector<int> m_State;
  bool m_Protected;

  void UpdateState();
  virtual bool PushLocal();

 public:
  GOCombination(GOCombinationDefinition& combination_template,
                GODefinitionFile* organfile);
  virtual ~GOCombination();
  int GetState(unsigned no);
  void SetState(unsigned no, int value);
  void Copy(GOCombination* combination);
  void Clear();
  GOCombinationDefinition* GetTemplate();
};

#endif
