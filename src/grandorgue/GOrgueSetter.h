/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GORGUESETTER_H
#define GORGUESETTER_H

#include "ptrvector.h"

class GOGUIPanel;
class GOGUILabel;
class GOrgueFrameGeneral;
class GOrgueMidiEvent;
class GOrgueSetterButton;
class GrandOrgueFile;
class IniFileConfig;

typedef enum 
{
	SETTER_REGULAR,
	SETTER_SCOPE,
	SETTER_SCOPED
} SetterType;

class GOrgueSetter
{
private:
	GrandOrgueFile* m_organfile;
	unsigned m_pos;
	ptr_vector<GOrgueFrameGeneral> m_framegeneral;
	ptr_vector<GOrgueSetterButton> m_button;
	GOGUILabel* m_PosDisplay;
	SetterType m_SetterType;

	void SetSetterType(SetterType type);

public:
	GOrgueSetter(GrandOrgueFile* organfile);
	virtual ~GOrgueSetter();

	GOGUIPanel* CreatePanel();
	void Load(IniFileConfig& cfg);
	void Save(IniFileConfig& cfg, bool prefix);
	void PreparePlayback();
	void Change(GOrgueSetterButton* button);
	void ProcessMidi(const GOrgueMidiEvent& event);

	bool IsSetterActive();
	void ToggleSetter();
	void SetterActive(bool on);
	SetterType GetSetterType();

	void Next();
	void Prev();
	void Push();
	unsigned GetPosition();
	void SetPosition(int pos, bool push = true);
};

#endif
