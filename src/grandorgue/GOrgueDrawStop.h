/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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

class GOrgueDrawstop : public GOrgueButton
{
private:
	int m_GCState;
	bool m_ActiveState;
	bool m_CombinationState;

protected:
	void SetState(bool on);
	virtual void ChangeState(bool on) = 0;

public:
	GOrgueDrawstop(GrandOrgueFile* organfile);
	void Load(GOrgueConfigReader& cfg, wxString group, wxString name = wxT(""));
	void Save(GOrgueConfigWriter& cfg);
	virtual void Set(bool on);
	virtual void PreparePlayback();
	void Reset();
	void SetCombination(bool on);

	bool IsActive() const;
};

#endif
