/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEELEMENTCREATOR_H
#define GORGUEELEMENTCREATOR_H

#include "GOrgueSetterButtonCallback.h"
#include "ptrvector.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueButton;
class GOrgueEnclosure;
class GOrgueLabel;
class GOrgueSetterButton;
class GrandOrgueFile;

struct ElementListEntry {
	wxString name;
	int value;
	bool is_public;
	bool is_pushbutton;
};

class GOrgueElementCreator : private GOrgueSetterButtonCallback
{
private:
	void SetterButtonChanged(GOrgueSetterButton* button);

protected:
	ptr_vector<GOrgueSetterButton> m_button;

	virtual const struct ElementListEntry* GetButtonList() = 0;
	virtual void ButtonChanged(int id) = 0;
	void CreateButtons(GrandOrgueFile* organfile);

public:
	GOrgueElementCreator();
	virtual ~GOrgueElementCreator();

	virtual void Load(GOrgueConfigReader& cfg) = 0;

	virtual GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel) = 0;
	virtual GOrgueLabel* GetLabel(const wxString& name, bool is_panel) = 0;
	virtual GOrgueButton* GetButton(const wxString& name, bool is_panel);
};

#endif
