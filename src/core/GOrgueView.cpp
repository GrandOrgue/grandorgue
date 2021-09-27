/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueView.h"

#include "GOrgueDocumentBase.h"

GOrgueView::GOrgueView(GOrgueDocumentBase* doc, wxWindow* wnd) :
	m_doc(doc),
	m_wnd(wnd)
{
}

GOrgueView::~GOrgueView()
{
	if (m_doc)
		m_doc->unregisterWindow(this);
	m_doc = NULL;
}

void GOrgueView::RemoveView()
{
	m_doc = NULL;
	m_wnd->Hide();
	m_wnd->Destroy();
}

void GOrgueView::ShowView()
{
	m_wnd->Show();
	m_wnd->Raise();
}

void GOrgueView::SyncState()
{
}
