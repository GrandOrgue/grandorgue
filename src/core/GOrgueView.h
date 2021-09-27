/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDIALOGVIEW_H
#define GORGUEDIALOGVIEW_H

#include <wx/window.h>

class GOrgueDocumentBase;

class GOrgueView
{
private:
	GOrgueDocumentBase* m_doc;
	wxWindow* m_wnd;

public:
	GOrgueView(GOrgueDocumentBase* doc, wxWindow* wnd);
	virtual ~GOrgueView();

	virtual void RemoveView();
	void ShowView();

	virtual void SyncState();
};

#endif
