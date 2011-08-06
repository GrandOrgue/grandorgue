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

#ifndef GORGUETREMULANT_H
#define GORGUETREMULANT_H

#include <wx/wx.h>
#include "GOrguePipe.h"
#include "GOrgueDrawStop.h"
#include "IniFileConfig.h"

class GOrgueTremulant : public GOrgueDrawstop
{

private:
	wxInt32 Period;
	wxInt16 StartRate;
	wxInt16 StopRate;
	wxInt16 AmpModDepth;
	GOrguePipe* m_pipe;

public:
	GOrgueTremulant();
	~GOrgueTremulant();
	void Load(IniFileConfig& cfg, wxString group, GOrgueDisplayMetrics* displayMetrics);
	void Save(IniFileConfig& cfg, bool prefix, wxString group);
	bool Set(bool on);

};

#endif /* GORGUETREMULANT_H_ */
