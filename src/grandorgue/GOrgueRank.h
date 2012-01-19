/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * MA 02111-1307, USA.
 */

#ifndef GORGUERANK_H
#define GORGUERANK_H

#include "GOrguePipeConfig.h"

class GOrguePipe;
class GOrgueTemperament;
class GOrgueStop;
class GrandOrgueFile;

class GOrgueRank : public GOrguePipeUpdateCallback
{
private:
	GrandOrgueFile* m_organfile;
        GOrgueStop* m_Stop;

public:
	GOrgueRank(GOrgueStop* stop, GrandOrgueFile* organfile);
	~GOrgueRank();
	GOrguePipe* GetPipe(unsigned index);
	unsigned GetPipeCount();
	void Abort();
	void PreparePlayback();
	GOrguePipeConfig& GetPipeConfig();
	void SetTemperament(const GOrgueTemperament& temperament);
	const wxString& GetName();

	void UpdateAmplitude();
	void UpdateTuning();
};

#endif
