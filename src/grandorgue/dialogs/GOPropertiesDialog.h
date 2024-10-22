/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPROPERTIES_H
#define GOPROPERTIES_H

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/string.h>

class GOOrganController;

class wxStaticLink : public wxStaticText {
private:
  wxString m_url;

  void OnClick(wxMouseEvent &event);

public:
  wxStaticLink(wxWindow *parent, const wxString &label, const wxString &url);
  DECLARE_EVENT_TABLE()
};

class GOProperties : public wxDialog {
private:
  GOOrganController *m_OrganController;

public:
  GOProperties(GOOrganController *organController, wxWindow *parent);
  ~GOProperties(void);
};

#endif
