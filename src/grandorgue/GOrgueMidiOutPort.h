/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEMIDIOUTPORT_H
#define GORGUEMIDIOUTPORT_H

#include <wx/string.h>
#include "ptrvector.h"

class GOrgueMidi;
class GOrgueMidiEvent;

class GOrgueMidiOutPort
{
protected:
	GOrgueMidi* m_midi;
	bool m_IsActive;
	wxString m_Name;
	wxString m_Prefix;
	unsigned m_ID;

	const wxString GetClientName();
	const wxString GetPortName();
	virtual void SendData(std::vector<unsigned char>& msg) = 0;

public:
	GOrgueMidiOutPort(GOrgueMidi* midi, wxString prefix, wxString name);
	virtual ~GOrgueMidiOutPort();

	virtual bool Open();
	virtual void Close() = 0;

	void Send(const GOrgueMidiEvent& e);

	const wxString& GetName();
	unsigned GetID();
	bool IsActive();
};

#endif
