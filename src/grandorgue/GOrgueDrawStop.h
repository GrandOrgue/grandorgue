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

#ifndef GORGUEDRAWSTOP_H
#define GORGUEDRAWSTOP_H

#include <wx/wx.h>
#include "GOrgueButton.h"
#include "GOrgueCombinationElement.h"

class GOrgueDrawstop : public GOrgueButton, public GOrgueCombinationElement
{
 public:
	typedef enum { FUNCTION_INPUT, FUNCTION_AND, FUNCTION_OR, FUNCTION_NOT, FUNCTION_XOR, FUNCTION_NAND, FUNCTION_NOR } GOrgueFunctionType;
private:
	static const struct IniFileEnumEntry m_function_types[];
	GOrgueFunctionType m_Type;
	int m_GCState;
	bool m_ActiveState;
	bool m_CombinationState;
	std::vector<GOrgueDrawstop*> m_ControlledDrawstops;
	std::vector<GOrgueDrawstop*> m_ControllingDrawstops;

protected:
	void SetState(bool on);
	virtual void ChangeState(bool on) = 0;

public:
	GOrgueDrawstop(GrandOrgueFile* organfile);
	void Init(GOrgueConfigReader& cfg, wxString group, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg);
	void RegisterControlled(GOrgueDrawstop* sw);
	virtual void Set(bool on);
	virtual void PreparePlayback();
	virtual void Update();
	void Reset();
	void SetCombination(bool on);

	bool IsActive() const;
	bool GetCombinationState() const;
};

#endif
