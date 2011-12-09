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

#ifndef GORGUEGENERAL_H
#define GORGUEGENERAL_H

#include "GOrguePushbutton.h"
#include "GOrgueFrameGeneral.h"

class IniFileConfig;

class GOrgueGeneral : public GOrguePushbutton
{
private:
	GOrgueFrameGeneral m_general;

public:
	GOrgueGeneral(GrandOrgueFile* organfile);
	void Load(IniFileConfig& cfg, wxString group);
	void LoadCombination(IniFileConfig& cfg);
	void Save(IniFileConfig& cfg, bool prefix);
	void Push();
	GOrgueFrameGeneral& GetGeneral();
};

#endif /* GORGUEGENERAL_H */
