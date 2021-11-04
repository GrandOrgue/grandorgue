/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOLOG_H
#define GOLOG_H

#include <wx/log.h>

class GOLogWindow;

class GOLog : public wxLog
{
private:
	GOLogWindow* m_LogWindow;

protected:
	void DoLogTextAtLevel (wxLogLevel level, const wxString &msg);

public:
	GOLog(wxWindow* parent);
	virtual ~GOLog();
};

#endif
