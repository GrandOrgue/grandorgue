/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUELOG_H
#define GORGUELOG_H

#include <wx/log.h>

class GOrgueLogWindow;

class GOrgueLog : public wxLog
{
private:
	GOrgueLogWindow* m_LogWindow;

protected:
	void DoLogTextAtLevel (wxLogLevel level, const wxString &msg);

public:
	GOrgueLog(wxWindow* parent);
	virtual ~GOrgueLog();
};

#endif
