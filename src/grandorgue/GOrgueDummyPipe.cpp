/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueDummyPipe.h"

GOrgueDummyPipe::GOrgueDummyPipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number) :
	GOrguePipe(organfile, rank, midi_key_number)
{
}

void GOrgueDummyPipe::Load(GOrgueConfigReader& cfg, wxString group, wxString prefix)
{
}

void GOrgueDummyPipe::Change(unsigned velocity, unsigned old_velocity)
{
}

