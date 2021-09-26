/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDUMMYPIPE_H
#define GORGUEDUMMYPIPE_H

#include "GOrguePipe.h"

class GOrgueDummyPipe : public GOrguePipe
{
private:

	void Change(unsigned velocity, unsigned old_velocity);

public:
	GOrgueDummyPipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number);

	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
};

#endif
