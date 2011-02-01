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

#pragma once
#include <wx/wx.h>
#include <vector>

class IniFileConfig;
class GOrgueStop;
class GOrgueCoupler;
class GOrgueDivisional;

class GOrgueManual 
{
public:
  GOrgueManual():
		Displayed(false),
	DispKeyColourInverted(false),
	DispKeyColourWooden(false),
	m_ManualNumber(0),m_Width(0),
	m_Height(0), m_X(0), m_Y(0),
	m_KeysY(0), m_PistonY(0),
	m_MIDI(),NumberOfLogicalKeys(0),
	FirstAccessibleKeyLogicalKeyNumber(0),
	FirstAccessibleKeyMIDINoteNumber(0),
	NumberOfAccessibleKeys(0),MIDIInputNumber(0),
	NumberOfStops(0),NumberOfCouplers(0),
	NumberOfDivisionals(0),NumberOfTremulants(0),
	tremulant(),Name(),	stop(NULL),
	coupler(NULL),divisional(NULL)
  {}

	void Load(IniFileConfig& cfg, const char* group);
	void Set(int note, bool on, bool pretend = false, int depth = 0, GOrgueCoupler* prev = 0);
	void MIDI(void);
	~GOrgueManual(void);

	bool Displayed : 1;
	bool DispKeyColourInverted : 1;
	bool DispKeyColourWooden : 1;

	wxInt16 m_ManualNumber;
	wxInt16 m_Width, m_Height, m_X, m_Y, m_KeysY, m_PistonY;
	wxInt16 m_MIDI[85];

	wxInt16 NumberOfLogicalKeys;
	wxInt16 FirstAccessibleKeyLogicalKeyNumber;
	wxInt16 FirstAccessibleKeyMIDINoteNumber;
	wxInt16 NumberOfAccessibleKeys;
	wxInt16 MIDIInputNumber;
	wxInt16 NumberOfStops;
	wxInt16 NumberOfCouplers;
	wxInt16 NumberOfDivisionals;
	wxInt16 NumberOfTremulants;
	wxInt16 tremulant[10];

	wxString Name;

  std::vector<GOrgueStop*> stop;
	GOrgueCoupler* coupler;
	GOrgueDivisional* divisional;
};

