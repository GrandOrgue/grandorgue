/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECTSDIALOG_H
#define GOMIDIOBJECTSDIALOG_H

#include <vector>

#include "common/GOSimpleDialog.h"
#include "document-base/GOView.h"

class wxButton;
class wxGridEvent;

class GOGrid;
class GOMidiObject;

class GOMidiObjectsDialog : public GOSimpleDialog, public GOView {
private:
  const std::vector<GOMidiObject *> &r_MidiObjects;

  GOGrid *m_ObjectsGrid;
  wxButton *m_ConfigureButton;
  wxButton *m_StatusButton;
  std::vector<wxButton *> m_ActionButtons;

public:
  GOMidiObjectsDialog(
    GODocumentBase *doc,
    wxWindow *parent,
    GODialogSizeSet &dialogSizes,
    const std::vector<GOMidiObject *> &midiObjects);

private:
  void ApplyAdditionalSizes(const GOAdditionalSizeKeeper &sizeKeeper) override;
  void CaptureAdditionalSizes(
    GOAdditionalSizeKeeper &sizeKeeper) const override;

  bool TransferDataToWindow() override;

  GOMidiObject *GetSelectedObject() const;
  void ConfigureSelectedObject();

  void OnSelectCell(wxGridEvent &event);
  void OnObjectDoubleClick(wxGridEvent &event) { ConfigureSelectedObject(); }
  void OnConfigureButton(wxCommandEvent &event) { ConfigureSelectedObject(); }
  void OnStatusButton(wxCommandEvent &event);
  void OnActionButton(wxCommandEvent &event);

  DECLARE_EVENT_TABLE()
};

#endif // GOMIDIOBJECTSDIALOG_H
