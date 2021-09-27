/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEGENERAL_H
#define GORGUEGENERAL_H

#include "GOrgueFrameGeneral.h"
#include "GOrguePushbutton.h"

class GOrgueConfigReader;

class GOrgueGeneral : public GOrguePushbutton
{
private:
	GOrgueFrameGeneral m_general;

public:
	GOrgueGeneral(GOrgueCombinationDefinition& general_template, GrandOrgueFile* organfile, bool is_setter);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Push();
	GOrgueFrameGeneral& GetGeneral();

	wxString GetMidiType();
};

#endif /* GORGUEGENERAL_H */
