/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPROPERTIESDIALOG_H
#define GOPROPERTIESDIALOG_H

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/string.h>

class GOOrganController;

class GOPropertiesDialog : public wxDialog {
private:
  GOOrganController *m_OrganController;

public:
  GOPropertiesDialog(GOOrganController *organController, wxWindow *parent);
};

#endif // GOPROPERTIESDIALOG_H
