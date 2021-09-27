/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef ORGANSELECTDIALOG_H_
#define ORGANSELECTDIALOG_H_

#include <wx/dialog.h>
#include <wx/listctrl.h>

class GOrgueOrgan;
class GOrgueOrganList;

class OrganSelectDialog : public wxDialog
{
private:
	const GOrgueOrganList& m_OrganList;
	wxListView* m_Organs;

	enum {
		ID_ORGANS = 200
	};

	void OnOK(wxCommandEvent& event);
	void OnDoubleClick(wxListEvent& event);

public:
	OrganSelectDialog(wxWindow* parent, wxString title, const GOrgueOrganList& organList);

	const GOrgueOrgan* GetSelection();

	DECLARE_EVENT_TABLE()
};

#endif
