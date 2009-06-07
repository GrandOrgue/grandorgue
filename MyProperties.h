/*
 * OurOrgan - free pipe organ simulator based on MyOrgan
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

class wxStaticLink : public wxStaticText
{
public:
    wxStaticLink(wxWindow* parent, const wxString& label, const wxString& url);
	void OnClick(wxMouseEvent& event);
	DECLARE_EVENT_TABLE()
protected:
    wxString m_url;
};

class MyProperties : public wxDialog
{
public:
	MyProperties(wxWindow* parent);
	~MyProperties(void);
};

class MyRegister : public wxDialog
{
public:
	MyRegister(wxWindow* parent);
	~MyRegister(void);
	void OnKey(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
    wxTextCtrl* m_key;
	DECLARE_EVENT_TABLE()
protected:
    enum {
        ID_LICENSEKEY = 300
    };
};
