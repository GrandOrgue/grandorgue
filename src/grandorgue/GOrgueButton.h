/*
 * GrandOrgue - free pipe organ simulator
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

#ifndef GORGUEBUTTON_H
#define GORGUEBUTTON_H

#include <wx/wx.h>
#include "GOrgueMidiReceiver.h"

class GOrgueMidiEvent;
class GrandOrgueFile;
class IniFileConfig;

class GOrgueButton 
{
protected:
	GrandOrgueFile* m_organfile;
	GOrgueMidiReceiver m_midi;
	bool m_Pushbutton;
	wxString m_group;
	bool m_Displayed;
	wxString m_Name;
	bool m_Engaged;
	bool m_DisplayInInvertedState;

public:
	GOrgueButton(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton);
	virtual ~GOrgueButton();
	void Load(IniFileConfig& cfg, wxString group, wxString name = wxT(""));
	virtual void Save(IniFileConfig& cfg, bool prefix);
	bool IsDisplayed();
	const wxString& GetName();
	GOrgueMidiReceiver& GetMidiReceiver();

	virtual void ProcessMidi(const GOrgueMidiEvent& event);
	virtual void Push();
	virtual void Set(bool on);
	virtual void Display(bool onoff);
	bool IsEngaged() const;
	bool DisplayInverted() const;
};

#endif
