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
#include "gui/dialogs/common/GOTabbedDialog.h"

#include "GOOrganSettingsButtonsProxy.h"

class wxButton;

class GOOrganModel;
class GOOrganSettingsPipesTab;

class GOOrganSettingsDialog : public GOTabbedDialog,
                              public GOView,
                              private GOOrganSettingsButtonsProxy::Listener {
private:
  GOOrganSettingsPipesTab *m_PipesTab;

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
