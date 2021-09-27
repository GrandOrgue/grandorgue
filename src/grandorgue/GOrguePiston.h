/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPISTON_H
#define GORGUEPISTON_H

#include "GOrgueControlChangedHandler.h"
#include "GOrguePushbutton.h"

class GOrgueButton;
class GOrgueConfigReader;
class GOrgueConfigWriter;

class GOrguePiston : public GOrguePushbutton, private GOrgueControlChangedHandler
{
private:
	GOrgueButton* drawstop;

	void ControlChanged(void* control);

public:

	GOrguePiston(GrandOrgueFile* organfile);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Push();

	wxString GetMidiType();
};

#endif /* GORGUEPISTON_H */
