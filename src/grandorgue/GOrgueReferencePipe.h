/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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
 */

#ifndef GORGUEREFERENCEPIPE_H
#define GORGUEREFERENCEPIPE_H

#include "GOrgueCacheObject.h"
#include "GOrguePipe.h"

class GOrgueReferencePipe : public GOrguePipe, private GOrgueCacheObject
{
private:
	GOrguePipe* m_Reference;
	unsigned m_ReferenceID;
	wxString m_Filename;

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(SHA_CTX& ctx);
	const wxString& GetLoadTitle();

	void Change(unsigned velocity, unsigned old_velocity);

public:
	GOrgueReferencePipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number);

	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
};

#endif
