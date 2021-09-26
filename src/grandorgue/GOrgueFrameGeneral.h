/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEFRAMEGENERAL_H
#define GORGUEFRAMEGENERAL_H

#include "GOrgueCombination.h"
#include "GOrgueSaveableObject.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GrandOrgueFile;

class GOrgueFrameGeneral : public GOrgueCombination, private GOrgueSaveableObject
{
private:
	GrandOrgueFile* m_organfile;
	bool m_IsSetter;

	void Save(GOrgueConfigWriter& cfg);
	void LoadCombination(GOrgueConfigReader& cfg);

public:
	GOrgueFrameGeneral(GOrgueCombinationDefinition& general_template, GrandOrgueFile* organfile, bool is_setter);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Push();
};

#endif /* GORGUEFRAMEGENERAL_H */
