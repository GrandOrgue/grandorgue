/*
 * GrandOrgue - a free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GRANDORGUE_H
#define GRANDORGUE_H

#include "GrandOrgueDef.h"

#include <wx/wx.h>
#include <wx/html/helpctrl.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/registry.h>
#endif

#include "GOrgueLCD.h"
#include "GOrgueSound.h"
#include "GOrgueMeter.h"
#include "GrandOrgueFrame.h"

class wxSingleInstanceChecker;
class wxSplashScreenWindow;
class wxDocManager;
class wxConfigBase;
class stServer;

class GOrgueApp : public wxApp
{
public:
  GOrgueApp();
  bool OnInit();
  int OnExit();
  void AsyncLoadFile(wxString iFile);
  GOrgueFrame* frame;
  wxDocManager* m_docManager;
  wxString m_path;
  wxHtmlHelpController* m_help;
protected:
  wxLocale m_locale;
  stServer* m_server;
  GOrgueSound* m_soundSystem;
  wxConfigBase* pConfig;
  wxSingleInstanceChecker* single_instance;
};


DECLARE_APP(GOrgueApp)

#endif
