/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

#ifndef GRANDORGUETOOL_H
#define GRANDORGUETOOL_H

#include <wx/app.h>
#include <wx/cmdline.h>

#include <vector>

class GOTool : public wxAppConsole {
private:
  wxLocale m_locale;
  static const wxCmdLineEntryDesc m_cmdLineDesc[];

  bool OnInit();
  int OnRun();
  void OnInitCmdLine(wxCmdLineParser &parser);
  bool OnCmdLineParsed(wxCmdLineParser &parser);

  bool CmdLineCreate(wxCmdLineParser &parser);

  bool CreateOrganPackage(
    wxString organPackage,
    wxString title,
    wxString inputDirectory,
    std::vector<wxString> odfs,
    std::vector<wxString> packages);

public:
  GOTool();
};

DECLARE_APP(GOTool)

#endif
