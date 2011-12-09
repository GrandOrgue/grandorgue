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
#include "GOrgueLabel.h"
#include "GOrgueEnclosure.h"

class GOGUIPanel;
class GOGUILabel;
class GOGUIControl;
class GOrgueFrameGeneral;
class GOrgueMidiEvent;
class GOrgueSetterButton;
class GrandOrgueFile;
class IniFileConfig;
struct IniFileEnumEntry;

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
	unsigned m_crescendopos;
	unsigned m_crescendobank;
	ptr_vector<GOrgueFrameGeneral> m_framegeneral;
	ptr_vector<GOrgueFrameGeneral> m_general;
	ptr_vector<GOrgueFrameGeneral> m_crescendo;
	ptr_vector<GOrgueSetterButton> m_button;
	GOrgueLabel m_PosDisplay;
	GOrgueLabel m_CrescendoDisplay;
	GOrgueEnclosure m_swell;
	SetterType m_SetterType;

	void SetSetterType(SetterType type);
	void SetCrescendoType(unsigned no);
	void Crescendo(int pos, bool force = false);

	static const struct IniFileEnumEntry m_setter_element_types[];

public:
	GOrgueSetter(GrandOrgueFile* organfile);
	virtual ~GOrgueSetter();

	GOGUIPanel* CreateCouplerPanel(IniFileConfig& cfg, unsigned manual);
	GOGUIPanel* CreateGeneralsPanel(IniFileConfig& cfg);
	GOGUIPanel* CreateSetterPanel(IniFileConfig& cfg);
	GOGUIPanel* CreateCrescendoPanel(IniFileConfig& cfg);
	GOGUIPanel* CreateDivisionalPanel(IniFileConfig& cfg);
	GOGUIControl* CreateGUIElement(IniFileConfig& cfg, wxString group, GOGUIPanel* panel);

	void Load(IniFileConfig& cfg);
	void Save(IniFileConfig& cfg, bool prefix);
	void LoadCombination(IniFileConfig& cfg);
	void PreparePlayback();
	void Change(GOrgueSetterButton* button);
	void ProcessMidi(const GOrgueMidiEvent& event);
	void ControlChanged(void* control);

	bool StoreInvisibleObjects();
	bool IsSetterActive();
	void ToggleSetter();
	void SetterActive(bool on);
	SetterType GetSetterType();

	void Next();
	void Prev();
	void Push();
	unsigned GetPosition();
	void SetPosition(int pos, bool push = true);
	void ResetDisplay();
};

#endif
