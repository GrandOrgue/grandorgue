/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESWITCH_H
#define GORGUESWITCH_H

#include "GOrgueDrawStop.h"

class GOrgueSwitch : public GOrgueDrawstop
{
protected:
	void ChangeState(bool);
	void SetupCombinationState();

public:
	GOrgueSwitch(GrandOrgueFile* organfile);
	~GOrgueSwitch();

	wxString GetMidiType();
};

#endif
