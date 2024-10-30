/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSELECTORGANDIALOG_H_
#define GOSELECTORGANDIALOG_H_

#include <vector>

#include <wx/event.h>

#include "common/GOSimpleDialog.h"

class wxGridEvent;

class GOConfig;
class GOGrid;
class GOOrgan;
class GOOrganList;

class GOSelectOrganDialog : public GOSimpleDialog {
private:
  const GOOrganList &r_OrganList;

  GOGrid *m_GridOrgans;
  std::vector<GOOrgan *> m_OrganPtrs;

  void ApplyAdditionalSizes(const GOAdditionalSizeKeeper &sizeKeeper) override;
  void CaptureAdditionalSizes(
    GOAdditionalSizeKeeper &sizeKeeper) const override;

  bool Validate() override;

  void OnSelectCell(wxGridEvent &event);
  void OnDoubleClick(wxGridEvent &event) { CloseAdvanced(wxID_OK); }

public:
  GOSelectOrganDialog(wxWindow *parent, GOConfig &config);

  bool TransferDataToWindow() override;

  const GOOrgan *GetSelection();

  DECLARE_EVENT_TABLE()
};

#endif
