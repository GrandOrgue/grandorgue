/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUECOMBINATIONDEFINITION_H
#define GORGUECOMBINATIONDEFINITION_H

#include <wx/wx.h>
#include <vector>
class GOrgueCombinationElement;
class GOrgueManual;
class GrandOrgueFile;

class GOrgueCombinationDefinition
{
public:
	typedef enum { COMBINATION_STOP = 0, COMBINATION_COUPLER = 1, COMBINATION_TREMULANT = 2, COMBINATION_DIVISIONALCOUPLER = 3 } CombinationType;
	typedef struct
	{
		CombinationType type;
		int manual;
		unsigned index;
		bool store_unconditional;
		wxString group;
		GOrgueCombinationElement* control;
	} CombinationSlot;
private:
	GrandOrgueFile* m_organfile;
	std::vector<CombinationSlot> m_Content;

public:
	GOrgueCombinationDefinition(GrandOrgueFile* organfile);
	~GOrgueCombinationDefinition();

	void InitGeneral();
	void InitDivisional(unsigned manual_number);

	int findEntry(CombinationType type, int manual, unsigned index);

	const std::vector<CombinationSlot>& GetCombinationElements();
};

#endif
