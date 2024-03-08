/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSELECTORGANDIALOG_H_
#define GOSELECTORGANDIALOG_H_

#include <wx/event.h>

#include "common/GOSimpleDialog.h"

class wxListEvent;
class wxListView;

class GOConfig;
class GOOrgan;
class GOOrganList;

class GOSelectOrganDialog : public GOSimpleDialog {
private:
  const GOOrganList &r_OrganList;
  wxListView *m_Organs;

  enum { ID_ORGANS = 200 };

  bool Validate() override;

  void OnDoubleClick(wxListEvent &event) { CloseAdvanced(wxID_OK); }

public:
  GOSelectOrganDialog(wxWindow *parent, GOConfig &config);

  bool TransferDataToWindow() override;

  const GOOrgan *GetSelection();

  DECLARE_EVENT_TABLE()
};

#endif
