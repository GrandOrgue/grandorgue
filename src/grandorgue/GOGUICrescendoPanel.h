/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOGUICRESCENDOPANEL_H
#define GOGUICRESCENDOPANEL_H

#include "GOGUIPanelCreator.h"

class GOGUIPanel;
class GrandOrgueFile;

class GOGUICrescendoPanel : public GOGUIPanelCreator
{
private:
	GrandOrgueFile* m_organfile;

	GOGUIPanel* CreateCrescendoPanel(GOrgueConfigReader& cfg);

public:
	GOGUICrescendoPanel(GrandOrgueFile* organfile);
	virtual ~GOGUICrescendoPanel();

	void CreatePanels(GOrgueConfigReader& cfg);
};

#endif
