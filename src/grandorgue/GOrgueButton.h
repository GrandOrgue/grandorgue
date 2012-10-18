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

#ifndef GORGUEBUTTON_H
#define GORGUEBUTTON_H

#include <wx/wx.h>
#include "GOrgueKeyReceiver.h"
#include "GOrgueMidiReceiver.h"
#include "GOrgueMidiSender.h"

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueMidiEvent;
class GrandOrgueFile;

class GOrgueButton 
{
protected:
	GrandOrgueFile* m_organfile;
	GOrgueMidiReceiver m_midi;
	GOrgueMidiSender m_sender;
	GOrgueKeyReceiver m_shortcut;
	bool m_Pushbutton;
	wxString m_group;
	bool m_Displayed;
	wxString m_Name;
	bool m_Engaged;
	bool m_DisplayInInvertedState;

public:
	GOrgueButton(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE midi_type, bool pushbutton);
	virtual ~GOrgueButton();
	void Load(GOrgueConfigReader& cfg, wxString group, wxString name = wxT(""));
	virtual void Save(GOrgueConfigWriter& cfg);
	bool IsDisplayed();
	const wxString& GetName();
	GOrgueMidiReceiver& GetMidiReceiver();
	GOrgueMidiSender& GetMidiSender();
	GOrgueKeyReceiver& GetKeyReceiver();

	virtual void ProcessMidi(const GOrgueMidiEvent& event);
	virtual void Push();
	virtual void Set(bool on);
	virtual void Display(bool onoff);
	void HandleKey(int key);
	bool IsEngaged() const;
	bool DisplayInverted() const;
	virtual void Abort();
	virtual void PreparePlayback();
};

#endif
