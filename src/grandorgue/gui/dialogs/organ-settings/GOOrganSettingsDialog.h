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

class wxButton;

class GOOrganModel;
class GOOrganSettingsEnclosuresTab;
class GOOrganSettingsPipesTab;

class GOOrganSettingsDialog : public GOOrganSettingsDialogBase, public GOView {
private:
  GOOrganSettingsPipesTab *m_PipesTab;
  GOOrganSettingsEnclosuresTab *m_EnclosuresTab;

  wxButton *m_AudioGroupAssistant;
  wxButton *m_Default;
  wxButton *m_Discard;
  wxButton *m_Apply;

public:
  GOOrganSettingsDialog(
    GOOrganModel &organModel, GODocumentBase *doc, wxWindow *parent);

private:
  void ButtonStatesChanged() override;

  void OnButtonDistributeAudio(wxCommandEvent &e);
  void OnButtonDefault(wxCommandEvent &e);
  void OnButtonDiscard(wxCommandEvent &e);
  void OnButtonApply(wxCommandEvent &e);

  DECLARE_EVENT_TABLE()
};

#endif /* GOORGANSETTINSDIALOG_H */
