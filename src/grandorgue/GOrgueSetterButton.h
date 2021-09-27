/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTERBUTTON_H
#define GORGUESETTERBUTTON_H

#include "GOrgueButton.h"

class GOrgueSetterButtonCallback;

class GOrgueSetterButton : public GOrgueButton
{
protected:
	GOrgueSetterButtonCallback* m_setter;

public:
	GOrgueSetterButton(GrandOrgueFile* organfile, GOrgueSetterButtonCallback* setter, bool Pushbutton);
	void Push(void);
	void Set(bool on);

	wxString GetMidiType();
};

#endif
