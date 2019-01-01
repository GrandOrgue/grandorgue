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

#ifndef GORGUECHOICE_H
#define GORGUECHOICE_H

#include <wx/choice.h>

template<class T>
class GOrgueChoice : public wxChoice
{
public:
	GOrgueChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize) :
	wxChoice(parent, id, pos, size)
	{
	}

	int Append (const wxString &item, const T value)
	{
		return wxChoice::Append(item, (void*)value);
	}

	T GetCurrentSelection()
	{
		return (T)(intptr_t)GetClientData(wxChoice::GetCurrentSelection());
	}

	void SetCurrentSelection(const T value)
	{
		SetSelection(0);
		for(unsigned i = 0; i < GetCount(); i++)
			if ((void*)value == GetClientData(i))
				SetSelection(i);
	}
};

#endif
