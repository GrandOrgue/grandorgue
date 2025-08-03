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
#include "midi/dialog-creator/GOMidiDialogListener.h"

class wxButton;
class wxGridEvent;
class wxGridRangeSelectEvent;

class GOConfig;
class GOGrid;
class GOMidiPlayingObject;
class GOOrganModel;

class GOMidiObjectsDialog : public GOSimpleDialog, public GOView {
private:
  GOConfig &r_config;

  wxString m_ExportImportDir;
  wxString m_OrganName;
  const std::vector<GOMidiPlayingObject *> &r_MidiObjects;

  class ObjectConfigListener : public GOMidiDialogListener {
  private:
    GOMidiObjectsDialog &r_dialog;
    unsigned m_index;
    GOMidiPlayingObject *p_object;

  public:
    ObjectConfigListener(
      GOMidiObjectsDialog &dialog,
      unsigned index,
      GOMidiPlayingObject *pObject);
    ~ObjectConfigListener();

    void OnSettingsApplied() override;
  };

  GOGrid *m_ObjectsGrid;
  wxButton *m_ConfigureButton;
  wxButton *m_StatusButton;
  std::vector<wxButton *> m_ActionButtons;
  std::vector<ObjectConfigListener> m_ObjectListeners;

  wxButton *m_ExportButton;
  wxButton *m_ImportButton;
  wxButton *m_ToInitialButton;

public:
  GOMidiObjectsDialog(
    GODocumentBase *doc, wxWindow *parent, GOOrganModel &organModel);

private:
  void ApplyAdditionalSizes(const GOAdditionalSizeKeeper &sizeKeeper) override;
  void CaptureAdditionalSizes(
    GOAdditionalSizeKeeper &sizeKeeper) const override;

  void RefreshIsConfigured(unsigned row, GOMidiPlayingObject *pObj);
  bool TransferDataToWindow() override;

  GOMidiPlayingObject *GetSelectedObject() const;
  void ConfigureSelectedObject();

  void OnSelectCell(wxGridEvent &event);
  void OnRangeSelect(wxGridRangeSelectEvent &event);
  void OnObjectDoubleClick(wxGridEvent &event) { ConfigureSelectedObject(); }
  void OnConfigureButton(wxCommandEvent &event) { ConfigureSelectedObject(); }
  void OnStatusButton(wxCommandEvent &event);
  void OnActionButton(wxCommandEvent &event);

  bool IsToImportSettingsFor(
    const wxString &fileName, const wxString &savedOrganName) const;
  wxString ExportMidiSettings(const wxString &fileName) const;

  template <typename ImportObjFun>
  void ImportAllObjects(ImportObjFun importObjFun);

  wxString ImportMidiSettings(const wxString &fileName);

  void OnExportButton(wxCommandEvent &event);
  void OnImportButton(wxCommandEvent &event);
  void OnToInitialButton(wxCommandEvent &event);

  void OnHide() override;

  DECLARE_EVENT_TABLE()
};

#endif // GOMIDIOBJECTSDIALOG_H
