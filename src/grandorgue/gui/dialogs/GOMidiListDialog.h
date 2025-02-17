/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDILISTDIALOG_H
#define GOMIDILISTDIALOG_H

#include <vector>

#include "common/GOSimpleDialog.h"
#include "document-base/GOView.h"

class wxButton;
class wxGridEvent;

class GOGrid;
class GOMidiObject;

class GOMidiListDialog : public GOSimpleDialog, public GOView {
private:
  const std::vector<GOMidiObject *> &r_MidiObjects;

  GOGrid *m_Objects;
  wxButton *m_Edit;
  wxButton *m_Status;
  std::vector<wxButton *> m_Buttons;

public:
  GOMidiListDialog(
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

  void OnObjectClick(wxGridEvent &event);
  void OnObjectDoubleClick(wxGridEvent &event);
  void OnEdit(wxCommandEvent &event);
  void OnStatus(wxCommandEvent &event);
  void OnButton(wxCommandEvent &event);

  DECLARE_EVENT_TABLE()
};

#endif
