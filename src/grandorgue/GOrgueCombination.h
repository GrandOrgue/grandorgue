/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECOMBINATION_H
#define GORGUECOMBINATION_H

#include <vector>

class GOrgueCombinationDefinition;
class GrandOrgueFile;

class GOrgueCombination
{
private:
	GrandOrgueFile* m_OrganFile;
protected:
	GOrgueCombinationDefinition& m_Template;
	std::vector<int> m_State;
	bool m_Protected;

	void UpdateState();
	virtual bool PushLocal();
public:
	GOrgueCombination(GOrgueCombinationDefinition& combination_template, GrandOrgueFile* organfile);
	virtual ~GOrgueCombination();
	int GetState(unsigned no);
	void SetState(unsigned no, int value);
	void Copy(GOrgueCombination* combination);
	void Clear();
	GOrgueCombinationDefinition* GetTemplate();
};

#endif
