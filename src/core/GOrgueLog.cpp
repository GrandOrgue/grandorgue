/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueLog.h"

#include "GOrgueLogWindow.h"

GOrgueLog::GOrgueLog(wxWindow* parent) :
	m_LogWindow(NULL)
{
	m_LogWindow = new GOrgueLogWindow(parent, wxID_ANY, _("Log messages"));
}

GOrgueLog::~GOrgueLog()
{
	if (m_LogWindow)
	{
		m_LogWindow = NULL;
	}
}

void GOrgueLog::DoLogTextAtLevel (wxLogLevel level, const wxString &msg)
{
	if (m_LogWindow)
		m_LogWindow->LogMsg(level, msg, time(0));
}
