/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOAPP_H
#define GOAPP_H

#include <wx/app.h>

class GOFrame;
class GOLog;
class GOSound;
class GOConfig;

class GOApp : public wxApp {
private:
  bool m_Restart;

  virtual void MacOpenFile(const wxString &fileName);
  void OnInitCmdLine(wxCmdLineParser &parser);
  bool OnCmdLineParsed(wxCmdLineParser &parser);
  bool OnInit();
  int OnRun();
  int OnExit();
  virtual void CleanUp() override;

protected:
  GOFrame *m_Frame;
  wxLocale m_locale;
  GOConfig *m_config;
  GOSound *m_soundSystem;
  GOLog *m_Log;
  wxString m_FileName;
  wxString m_InstanceName;
  bool m_IsGuiOnly;

public:
  GOApp();
  void SetRestart();
};

DECLARE_APP(GOApp)

#endif
