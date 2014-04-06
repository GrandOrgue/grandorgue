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

#ifndef GORGUETEMPERAMENTLIST_H
#define GORGUETEMPERAMENTLIST_H

#include "ptrvector.h"
#include <wx/string.h>

class GOrgueTemperament;

class GOrgueTemperamentList
{
private:
	ptr_vector<GOrgueTemperament> m_Temperaments;

	void InitTemperaments();

public:
	GOrgueTemperamentList();
	~GOrgueTemperamentList();

	const GOrgueTemperament& GetTemperament(wxString Name);
	const GOrgueTemperament& GetTemperament(unsigned index);
	unsigned GetTemperamentIndex(wxString name);
	const wxString GetTemperamentName(unsigned index);
	unsigned GetTemperamentCount();
};

#endif

