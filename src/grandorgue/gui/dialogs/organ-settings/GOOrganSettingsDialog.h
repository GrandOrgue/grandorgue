/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOORGANSETTINSDIALOG_H
#define GOORGANSETTINSDIALOG_H

#include <wx/event.h>

#include "document-base/GOView.h"

#include "GOOrganSettingsDialogBase.h"
#include "GOOrganSettingsTab.h"

class wxBookCtrlEvent;
class wxButton;

class GOOrganModel;
class GOOrganSettingsEnclosuresTab;
class GOOrganSettingsPipesTab;

class GOOrganSettingsDialog : public GOOrganSettingsDialogBase, public GOView {
private:
  wxButton *m_AudioGroupAssistant;
  wxButton *m_Default;
  wxButton *m_Discard;
  wxButton *m_Apply;

  GOOrganSettingsPipesTab *m_PipesTab;
  GOOrganSettingsEnclosuresTab *m_EnclosuresTab;

public:
  GOOrganSettingsDialog(
    GOOrganModel &organModel, GODocumentBase *doc, wxWindow *parent);

private:
  void ButtonStatesChanged() override;

  void OnTabSelecting(wxBookCtrlEvent &e);
  void OnTabSelected(wxBookCtrlEvent &e) { ButtonStatesChanged(); }

  template <typename T> T CallTabFunc(T (GOOrganSettingsTab::*pFunc)());

  void OnButtonDistributeAudio(wxCommandEvent &e) {
    CallTabFunc(&GOOrganSettingsTab::DistributeAudio);
  }

  void OnButtonDefault(wxCommandEvent &e) {
    CallTabFunc(&GOOrganSettingsTab::ResetToDefault);
  }

  void OnButtonDiscard(wxCommandEvent &e) {
    CallTabFunc(&GOOrganSettingsTab::DiscardChanges);
  }

  void OnButtonApply(wxCommandEvent &e) {
    CallTabFunc(&GOOrganSettingsTab::ApplyChanges);
  }

  DECLARE_EVENT_TABLE()
};

#endif /* GOORGANSETTINSDIALOG_H */
