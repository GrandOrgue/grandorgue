/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGENERAL_H
#define GOGENERAL_H

#include "GOFrameGeneral.h"
#include "GOPushbutton.h"

class GOConfigReader;

class GOGeneral : public GOPushbutton
{
private:
	GOFrameGeneral m_general;

public:
	GOGeneral(GOCombinationDefinition& general_template, GODefinitionFile* organfile, bool is_setter);
	void Load(GOConfigReader& cfg, wxString group);
	void Push();
	GOFrameGeneral& GetGeneral();

	wxString GetMidiType();
};

#endif /* GOGENERAL_H */
