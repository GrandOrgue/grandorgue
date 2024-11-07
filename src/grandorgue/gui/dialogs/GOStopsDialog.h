/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTOPSDIALOG_H
#define GOSTOPSDIALOG_H

#include <unordered_map>

#include "common/GOSimpleDialog.h"
#include "control/GOControlChangedHandler.h"
#include "document-base/GOView.h"

class wxCheckBox;

class GOCombinationElement;
class GOOrganModel;

class GOStopsDialog : public GOSimpleDialog,
                      public GOView,
                      private GOControlChangedHandler {
private:
  GOOrganModel &r_model;

  std::unordered_map<GOCombinationElement *, wxCheckBox *>
    mp_checkboxesByControl;

  void OnElementChanging(wxCommandEvent &event);
  void ControlChanged(GOControl *pControl) override;

  void OnShow() override;
  void OnHide() override;

public:
  GOStopsDialog(
    GODocumentBase *doc,
    wxWindow *parent,
    GODialogSizeSet &dialogSizes,
    GOOrganModel &model);

  DECLARE_EVENT_TABLE()
};

#endif /* GOSTOPSDIALOG_H */
