/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GRANDORGUE_H
#define GRANDORGUE_H

#include "GrandOrgueDef.h"

#include <wx/wx.h>
#include <wx/cmdline.h>

class wxSplashScreenWindow;
class wxDocManager;

class GOrgueFrame;
class GOrgueLog;
class GOrgueSound;
class GOrgueSettings;

class GOrgueApp : public wxApp
{
private:
	virtual void MacOpenFile(const wxString &fileName);

protected:
	GOrgueFrame* m_Frame;
	wxLocale m_locale;
	GOrgueSettings* m_Settings;
	GOrgueSound* m_soundSystem;
	wxDocManager* m_docManager;
	GOrgueLog* m_Log;
	wxString m_FileName;
	wxString m_InstanceName;

	static const wxCmdLineEntryDesc m_cmdLineDesc [];

public:
	GOrgueApp();
	bool OnInit();
	int OnRun();
	int OnExit();
	void AsyncLoadFile(wxString iFile);
	void OnInitCmdLine(wxCmdLineParser& parser);
	bool OnCmdLineParsed(wxCmdLineParser& parser);
};


DECLARE_APP(GOrgueApp)

#endif
