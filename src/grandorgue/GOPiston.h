/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOPISTON_H
#define GOPISTON_H

#include "GOControlChangedHandler.h"
#include "GOPushbutton.h"

class GOButton;
class GOConfigReader;
class GOConfigWriter;

class GOPiston : public GOPushbutton, private GOControlChangedHandler
{
private:
	GOButton* drawstop;

	void ControlChanged(void* control);

public:

	GOPiston(GODefinitionFile* organfile);
	void Load(GOConfigReader& cfg, wxString group);
	void Push();

	wxString GetMidiType();
};

#endif /* GOPISTON_H */
