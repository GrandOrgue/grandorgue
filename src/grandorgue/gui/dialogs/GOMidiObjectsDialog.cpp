/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObjectstDialog.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

#include "config/GOConfigFileReader.h"
#include "config/GOConfigFileWriter.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigReaderDB.h"
#include "config/GOConfigWriter.h"
#include "gui/size/GOAdditionalSizeKeeperProxy.h"
#include "gui/wxcontrols/GOGrid.h"
#include "midi/objects/GOMidiObjectContext.h"
#include "midi/objects/GOMidiPlayingObject.h"
#include "model/GOOrganModel.h"
#include "yaml/GOYamlModel.h"
#include "yaml/go-wx-yaml.h"

#include "GODocument.h"
#include "GOEvent.h"
#include "go-message-boxes.h"

enum {
  ID_LIST = 200,
  ID_EDIT,
  ID_STATUS,
  ID_BUTTON_EXPORT,
  ID_BUTTON_IMPORT,
  ID_BUTTON,
  ID_BUTTON_LAST = ID_BUTTON + 2,
};

BEGIN_EVENT_TABLE(GOMidiObjectsDialog, GOSimpleDialog)
EVT_GRID_CMD_SELECT_CELL(ID_LIST, GOMidiObjectsDialog::OnSelectCell)
EVT_GRID_CMD_CELL_LEFT_DCLICK(ID_LIST, GOMidiObjectsDialog::OnObjectDoubleClick)
EVT_BUTTON(ID_EDIT, GOMidiObjectsDialog::OnConfigureButton)
EVT_BUTTON(ID_STATUS, GOMidiObjectsDialog::OnStatusButton)
EVT_BUTTON(ID_BUTTON_EXPORT, GOMidiObjectsDialog::OnExportButton)
EVT_BUTTON(ID_BUTTON_IMPORT, GOMidiObjectsDialog::OnImportButton)
EVT_COMMAND_RANGE(
  ID_BUTTON, ID_BUTTON_LAST, wxEVT_BUTTON, GOMidiObjectsDialog::OnActionButton)
END_EVENT_TABLE()

enum {
  GRID_COL_TYPE = 0,
  GRID_COL_ELEMENT,
  GRID_COL_PATH,
  GRID_COL_CONFIGURED,
  GRID_N_COLS,
};

static wxString concat_file_specs(const wxString &one, const wxString &two) {
  return wxString::Format("%s|%s", one, two);
}

static wxString concat_file_spec(const wxString &descFmt, const wxString &pat) {
  return concat_file_specs(wxString::Format(descFmt, pat), pat);
}

static const wxString WX_GRID_MIDI_OBJECTS = wxT("GridMidiObjects");
static const wxString WX_MIDI_SETTINGS = wxT("MIDI Settings");
static const wxString WX_MIDI_SETTINGS_EXT_PURE = wxT("yaml");
static const wxString WX_MIDI_SETTINGS_EXT
  = wxT(".") + WX_MIDI_SETTINGS_EXT_PURE;
static const wxString WX_MIDI_SETTINGS_FILE_PAT
  = wxT("*") + WX_MIDI_SETTINGS_EXT;
static const wxString WX_MIDI_FILES_SPEC
  = concat_file_spec(_("Midi Settings files (%s)"), WX_MIDI_SETTINGS_FILE_PAT);
static const wxString WX_CMB_FILE_PAT = wxT("*.cmb");
static const wxString WX_CMB_FILES_SPEC
  = concat_file_spec(_("Settings files (%s)"), WX_CMB_FILE_PAT);
static const wxString WX_IMPORT_SPEC
  = concat_file_specs(WX_MIDI_FILES_SPEC, WX_CMB_FILES_SPEC);
static const wxString WX_ERROR = _("Error");
static const wxString WX_ORGAN = wxT("Organ");
static const wxString WX_ORGAN_NAME = wxT("ChurchName");
static const wxString WX_OBJECTS = wxT("objects");
static const wxString WX_PATH = wxT("path");

GOMidiObjectsDialog::ObjectConfigListener::ObjectConfigListener(
  GOMidiObjectsDialog &dialog, unsigned index, GOMidiPlayingObject *pObject)
  : r_dialog(dialog), m_index(index), p_object(pObject) {
  p_object->AddListener(this);
}

GOMidiObjectsDialog::ObjectConfigListener::~ObjectConfigListener() {
  p_object->RemoveListener(this);
}

void GOMidiObjectsDialog::ObjectConfigListener::OnSettingsApplied() {
  r_dialog.RefreshIsConfigured(m_index, p_object);
}

GOMidiObjectsDialog::GOMidiObjectsDialog(
  GODocumentBase *doc, wxWindow *parent, GOOrganModel &organModel)
  : GOSimpleDialog(
    parent,
    wxT("MIDI Objects"),
    _("MIDI Objects"),
    organModel.GetConfig().m_DialogSizes,
    wxEmptyString,
    0,
    wxCLOSE | wxHELP),
    GOView(doc, this),
    m_ExportImportDir(organModel.GetConfig().ExportImportPath()),
    m_OrganName(organModel.GetOrganName()),
    r_MidiObjects(organModel.GetMidiObjects()) {
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->AddSpacer(5);

  m_ObjectsGrid
    = new GOGrid(this, ID_LIST, wxDefaultPosition, wxSize(350, 200));
  m_ObjectsGrid->CreateGrid(0, GRID_N_COLS, wxGrid::wxGridSelectRows);
  m_ObjectsGrid->HideRowLabels();
  m_ObjectsGrid->EnableEditing(false);
  m_ObjectsGrid->SetColLabelValue(GRID_COL_TYPE, _("Type"));
  m_ObjectsGrid->SetColLabelValue(GRID_COL_ELEMENT, _("Element"));
  m_ObjectsGrid->SetColLabelValue(GRID_COL_PATH, _("Path"));
  m_ObjectsGrid->SetColLabelValue(GRID_COL_CONFIGURED, _("Configured"));
  m_ObjectsGrid->SetColSize(GRID_COL_TYPE, 100);
  m_ObjectsGrid->SetColSize(GRID_COL_ELEMENT, 100);
  m_ObjectsGrid->SetColSize(GRID_COL_PATH, 200);
  m_ObjectsGrid->SetColSize(GRID_COL_CONFIGURED, 30);

  topSizer->Add(m_ObjectsGrid, 1, wxEXPAND | wxALL, 5);

  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  for (unsigned id = ID_BUTTON; id <= ID_BUTTON_LAST; id++) {
    wxButton *button = new wxButton(this, id, wxEmptyString);
    sizer->Add(button, 0, id == ID_BUTTON ? wxRESERVE_SPACE_EVEN_IF_HIDDEN : 0);
    button->Hide();
    m_ActionButtons.push_back(button);
  }
  topSizer->Add(sizer, 0, wxALIGN_LEFT | wxALL, 1);

  wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
  m_ConfigureButton = new wxButton(this, ID_EDIT, _("C&onfigure..."));
  m_ConfigureButton->Disable();
  buttons->Add(m_ConfigureButton);
  m_StatusButton = new wxButton(this, ID_STATUS, _("&Status"));
  m_StatusButton->Disable();
  buttons->Add(m_StatusButton);
  topSizer->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 1);

  topSizer->AddSpacer(5);

  // add a custom button 'Reason into the space of the standard dialog button
  wxSizer *const pButtonSizer = GetButtonSizer();

  if (pButtonSizer) {
    pButtonSizer->InsertSpacer(2, 10);
    m_ExportButton = new wxButton(this, ID_BUTTON_EXPORT, _("Export"));
    pButtonSizer->Insert(
      3, m_ExportButton, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    m_ImportButton = new wxButton(this, ID_BUTTON_IMPORT, _("Import"));
    pButtonSizer->Insert(
      4, m_ImportButton, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 2);
    pButtonSizer->InsertSpacer(6, 10);
  }

  LayoutWithInnerSizer(topSizer);
}

void GOMidiObjectsDialog::ApplyAdditionalSizes(
  const GOAdditionalSizeKeeper &sizeKeeper) {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    const_cast<GOAdditionalSizeKeeper &>(sizeKeeper), WX_GRID_MIDI_OBJECTS);

  m_ObjectsGrid->ApplyColumnSizes(proxyMidiObjects);
}

void GOMidiObjectsDialog::CaptureAdditionalSizes(
  GOAdditionalSizeKeeper &sizeKeeper) const {
  GOAdditionalSizeKeeperProxy proxyMidiObjects(
    sizeKeeper, WX_GRID_MIDI_OBJECTS);

  m_ObjectsGrid->CaptureColumnSizes(proxyMidiObjects);
}

void GOMidiObjectsDialog::RefreshIsConfigured(
  unsigned row, GOMidiPlayingObject *pObj) {
  m_ObjectsGrid->SetCellValue(
    row, GRID_COL_CONFIGURED, pObj->IsMidiConfigured() ? _("Yes") : _("No"));
}

bool GOMidiObjectsDialog::TransferDataToWindow() {
  unsigned oldRowCnt = m_ObjectsGrid->GetNumberRows();
  unsigned newRowCnt = r_MidiObjects.size();

  m_ObjectListeners.clear();
  if (oldRowCnt)
    m_ObjectsGrid->DeleteRows(0, oldRowCnt);

  m_ObjectListeners.reserve(newRowCnt);
  m_ObjectsGrid->AppendRows(newRowCnt);
  for (unsigned i = 0; i < newRowCnt; i++) {
    GOMidiPlayingObject *pObj = r_MidiObjects[i];

    m_ObjectListeners.emplace_back(*this, i, pObj);
    pObj->AddListener(&m_ObjectListeners[i]);
    m_ObjectsGrid->SetCellValue(i, GRID_COL_TYPE, pObj->GetMidiTypeName());
    m_ObjectsGrid->SetCellValue(i, GRID_COL_ELEMENT, pObj->GetName());
    m_ObjectsGrid->SetCellValue(i, GRID_COL_PATH, pObj->GetPath());
    RefreshIsConfigured(i, pObj);
  }
  return true;
}

GOMidiPlayingObject *GOMidiObjectsDialog::GetSelectedObject() const {
  return r_MidiObjects[m_ObjectsGrid->GetGridCursorRow()];
}

void GOMidiObjectsDialog::ConfigureSelectedObject() {
  int row = m_ObjectsGrid->GetGridCursorRow();
  GODocument *pDoc = dynamic_cast<GODocument *>(GetDocument());

  if (row >= 0) {
    GOMidiPlayingObject *pObj = r_MidiObjects[row];

    pDoc->ShowMIDIEventDialog(*pObj, pObj);
  }
}

void GOMidiObjectsDialog::OnSelectCell(wxGridEvent &event) {
  int index = event.GetRow();
  bool isAnySelected = index >= 0;

  m_ConfigureButton->Enable(isAnySelected);
  m_StatusButton->Enable(isAnySelected);
  if (isAnySelected) {
    m_ObjectsGrid->SelectRow(index);
    GOMidiPlayingObject *obj = r_MidiObjects[index];
    std::vector<wxString> actions = obj->GetElementActions();

    for (unsigned i = 0; i < m_ActionButtons.size(); i++)
      if (i < actions.size()) {
        m_ActionButtons[i]->SetLabel(actions[i]);
        m_ActionButtons[i]->Show();
      } else
        m_ActionButtons[i]->Hide();
  }
  Layout();
  event.StopPropagation();
}

void GOMidiObjectsDialog::OnStatusButton(wxCommandEvent &event) {
  GOMidiPlayingObject *obj = GetSelectedObject();
  wxString status = obj->GetElementStatus();

  GOMessageBox(
    wxString::Format(_("Status: %s"), status),
    obj->GetMidiTypeName() + _(" ") + obj->GetName(),
    wxOK);
}

void GOMidiObjectsDialog::OnActionButton(wxCommandEvent &event) {
  GOMidiPlayingObject *obj = GetSelectedObject();
  obj->TriggerElementActions(event.GetId() - ID_BUTTON);
}

wxString GOMidiObjectsDialog::ExportMidiSettings(
  const wxString &fileName) const {
  GOYamlModel::Out yamlOut(m_OrganName, WX_MIDI_SETTINGS);
  YAML::Node &rootNode = yamlOut.m_GlobalNode;
  YAML::Node objectsNode;

  for (const auto pObj : r_MidiObjects) {
    YAML::Node objNode;

    pObj->ToYaml(objNode);
    if (!objNode.IsNull())
      objectsNode.push_back(objNode);
  }
  put_to_map_if_not_null(rootNode, WX_OBJECTS, objectsNode);

  const wxString errMsg = yamlOut.writeTo(fileName);

  return errMsg;
}

/**
 * Check the organName of the imported midi settings file. If it differs from
 * the current organ name then ask for the user
 * @param fileName the midi setting file name
 * @param organName the organ the midi settings file was saved of
 * @return true if the churchNames are the same or the user agree with importing
 *   the midi settings file
 */
bool GOMidiObjectsDialog::IsToImportSettingsFor(
  const wxString &fileName, const wxString &savedOrganName) const {
  bool isToImport = true;

  if (savedOrganName != m_OrganName) {
    wxLogWarning(
      _("This MIDI settings file '%s' was originally made for another organ "
        "'%s'"),
      fileName,
      savedOrganName);
    isToImport = wxMessageBox(
                   wxString::Format(
                     _("This MIDI settings file '%s' was originally made for "
                       "another organ '%s'. Importing it can cause various "
                       "problems. Should it really be imported?"),
                     fileName,
                     savedOrganName),
                   _("Import MIDI Settings"),
                   wxYES_NO,
                   nullptr)
      == wxYES;
  }
  return isToImport;
}

template <typename ImportObjFun>
void GOMidiObjectsDialog::ImportAllObjects(ImportObjFun importObjFun) {
  for (unsigned l = r_MidiObjects.size(), i = 0; i < l; i++) {
    const auto pObj = r_MidiObjects[i];

    importObjFun(*pObj);
    RefreshIsConfigured(i, pObj);
  }
}

wxString GOMidiObjectsDialog::ImportMidiSettings(const wxString &fileName) {
  wxString errMsg;
  const wxString fileExt = wxFileName(fileName).GetExt();

  try {
    if (fileExt == WX_MIDI_SETTINGS_EXT_PURE) {
      GOYamlModel::In yamlIn(m_OrganName, fileName, WX_MIDI_SETTINGS);
      const YAML::Node &rootNode = yamlIn.m_GlobalNode;

      if (is_to_import_to_this_organ(
            m_OrganName,
            WX_MIDI_SETTINGS,
            fileName,
            yamlIn.GetFileOrganName())) {
        YAML::Node objectsNode = rootNode[WX_OBJECTS];

        if (objectsNode.IsNull())
          throw wxString::Format(
            _("There is no %s node in the yaml file"), WX_OBJECTS);
        if (!objectsNode.IsSequence())
          throw wxString::Format(
            _("The %s node in not a sequence"), WX_OBJECTS);

        GOStringSet usedYamlPaths;
        std::unordered_map<wxString, YAML::Node, wxStringHash, wxStringEqual>
          nodesByPath;

        for (unsigned l = objectsNode.size(), i = 0; i < l; i++) {
          const YAML::Node &objNode = objectsNode[i];
          const wxString yamlPath = wxString::Format("%s[%u]", WX_OBJECTS, i);
          const wxString objectPath
            = read_string(objNode, yamlPath, WX_PATH, true, usedYamlPaths);

          if (!objectPath.IsEmpty()) {
            if (nodesByPath.find(objectPath) == nodesByPath.end())
              nodesByPath[objectPath] = objNode;
            else
              wxLogError(_("Duplicate object with the path %s"), objectPath);
          }
        }

        GOStringSet usedobjectPaths;
        YAML::Node emptyNode;

        ImportAllObjects(
          [&nodesByPath, &usedobjectPaths, &emptyNode](GOMidiObject &obj) {
            const wxString objectPath = obj.GetPath();
            const auto it = nodesByPath.find(objectPath);

            if (it != nodesByPath.end()) {
              obj.FromYaml(it->second, objectPath);
              usedobjectPaths.insert(objectPath);
            } else
              obj.FromYaml(emptyNode, objectPath);
          });

        for (const auto &e : nodesByPath) {
          const wxString &objPath = e.first;

          if (usedobjectPaths.find(objPath) == usedobjectPaths.end())
            wxLogWarning(_("Unused MIDI object path: %s"), objPath);
        }
      }
    } else {
      GOConfigFileReader cmbFile;

      if (!cmbFile.Read(fileName))
        throw wxString::Format(_("Unable to read '%s'"), fileName);

      GOConfigReaderDB ini;
      ini.ReadData(cmbFile, CMBSetting, false);
      GOConfigReader cfg(ini);

      wxString savedOrganName
        = cfg.ReadString(CMBSetting, WX_ORGAN, WX_ORGAN_NAME);

      if (is_to_import_to_this_organ(
            m_OrganName, WX_MIDI_SETTINGS, fileName, savedOrganName))
        ImportAllObjects(
          [&cfg](GOMidiObject &obj) { obj.LoadMidiSettings(cfg); });
    }
  } catch (const wxString &error) {
    errMsg = error;
  } catch (const std::exception &e) {
    errMsg = e.what();
  } catch (...) { // We must not allow unhandled exceptions here
    errMsg.Printf("Unknown exception");
  }
  return errMsg;
}

void GOMidiObjectsDialog::OnExportButton(wxCommandEvent &event) {
  wxFileDialog dlg(
    this,
    _("Export MIDI Settings"),
    m_ExportImportDir,
    m_OrganName + WX_MIDI_SETTINGS_EXT,
    WX_MIDI_FILES_SPEC,
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dlg.ShowModal() == wxID_OK) {
    wxString exportedFilePath = dlg.GetPath();

    if (!exportedFilePath.EndsWith(WX_MIDI_SETTINGS_EXT, nullptr))
      exportedFilePath += WX_MIDI_SETTINGS_EXT;
    const wxString errMsg = ExportMidiSettings(exportedFilePath);

    if (!errMsg.IsEmpty()) {
      wxLogError(errMsg);
      GOMessageBox(
        wxString::Format(
          _("Failed to export MIDI settings to '%s': %s"),
          exportedFilePath,
          errMsg),
        WX_ERROR,
        wxOK | wxICON_ERROR,
        this);
    }
  }
}

void GOMidiObjectsDialog::OnImportButton(wxCommandEvent &event) {
  wxFileDialog dlg(
    this,
    _("Import Midi Settings"),
    m_ExportImportDir,
    m_OrganName + WX_MIDI_SETTINGS_EXT,
    WX_IMPORT_SPEC,
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() == wxID_OK) {
    const wxString &importedFilePath = dlg.GetPath();
    const wxString errMsg = ImportMidiSettings(importedFilePath);

    if (!errMsg.IsEmpty()) {
      wxLogError(errMsg);
      GOMessageBox(
        wxString::Format(
          _("Failed to import MIDI settings from '%s': %s"),
          importedFilePath,
          errMsg),
        WX_ERROR,
        wxOK | wxICON_ERROR,
        this);
    }
  }
}

void GOMidiObjectsDialog::OnHide() {
  // remove references to the organ objects before destructing
  m_ObjectListeners.clear();
  GOSimpleDialog::OnHide();
}
