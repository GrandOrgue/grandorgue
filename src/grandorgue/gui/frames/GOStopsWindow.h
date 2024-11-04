/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTOPSWINDOW_H
#define GOSTOPSWINDOW_H

#include <unordered_map>

#include <wx/frame.h>

#include "control/GOControlChangedHandler.h"
#include "document-base/GOView.h"

class wxCheckBox;

class GOCombinationElement;
class GODialogSizeSet;
class GOOrganModel;
class GOSizeKeeper;

class GOStopsWindow : public wxFrame,
                      public GOView,
                      private GOControlChangedHandler {
private:
  GOSizeKeeper &r_SizeKeeper;
  GOOrganModel &r_model;

  std::unordered_map<GOCombinationElement *, wxCheckBox *>
    mp_checkboxesByControl;

  virtual void OnShow(wxShowEvent &event);

  void OnElementChanging(wxCommandEvent &event);
  void ControlChanged(GOControl *pControl) override;
  void SyncState() override;

public:
  GOStopsWindow(
    GODocumentBase *doc,
    wxWindow *parent,
    GODialogSizeSet &dialogSizes,
    GOOrganModel &model);

  DECLARE_EVENT_TABLE()
};

#endif /* GOSTOPSWINDOW_H */
