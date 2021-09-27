/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPROPERTIES_H
#define GORGUEPROPERTIES_H

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/string.h>

class GrandOrgueFile;

class wxStaticLink : public wxStaticText
{
private:
	wxString m_url;

	void OnClick(wxMouseEvent& event);

public:
	wxStaticLink(wxWindow* parent, const wxString& label, const wxString& url);
	DECLARE_EVENT_TABLE()
};

class GOrgueProperties : public wxDialog
{
private:
	GrandOrgueFile* m_organfile;

public:
	GOrgueProperties(GrandOrgueFile* organfile, wxWindow* parent);
	~GOrgueProperties(void);
};

#endif
