/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIPanel.h"

#include <wx/image.h>

#include "combinations/GOSetter.h"
#include "combinations/control/GODivisionalButtonControl.h"
#include "combinations/control/GOGeneralButtonControl.h"
#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "control/GOPistonControl.h"
#include "model/GOCoupler.h"
#include "model/GODivisionalCoupler.h"
#include "model/GOManual.h"
#include "model/GOStop.h"
#include "model/GOSwitch.h"
#include "model/GOTremulant.h"
#include "primitives/GODC.h"

#include "GOGUIButton.h"
#include "GOGUIControl.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUIEnclosure.h"
#include "GOGUIHW1Background.h"
#include "GOGUIHW1DisplayMetrics.h"
#include "GOGUIImage.h"
#include "GOGUILabel.h"
#include "GOGUILayoutEngine.h"
#include "GOGUIManual.h"
#include "GOGUIManualBackground.h"
#include "GOGUIPanelView.h"
#include "GOGUIPanelWidget.h"
#include "GOOrganController.h"
#include "Images.h"

GOGUIPanel::GOGUIPanel(GOOrganController *organController)
  : m_OrganController(organController),
    m_MouseState(organController->GetMouseState()),
    m_controls(0),
    m_WoodImages(0),
    m_BackgroundControls(0),
    m_Name(),
    m_GroupName(),
    m_metrics(0),
    m_layout(0),
    m_view(0),
    m_InitialOpenWindow(false) {
  for (unsigned i = 0; i < 64; i++)
    m_WoodImages.push_back(LoadBitmap(
      wxString::Format(wxT(GOBitmapPrefix "wood%02d"), i + 1), wxEmptyString));
}

GOGUIPanel::~GOGUIPanel() {
  if (m_layout)
    delete m_layout;
  if (m_metrics)
    delete m_metrics;
}

bool GOGUIPanel::InitialOpenWindow() { return m_InitialOpenWindow; }

GOOrganController *GOGUIPanel::GetOrganFile() { return m_OrganController; }

const wxString &GOGUIPanel::GetName() { return m_Name; }

const wxString &GOGUIPanel::GetGroupName() { return m_GroupName; }

GOBitmap GOGUIPanel::LoadBitmap(wxString filename, wxString maskname) {
  return m_OrganController->GetBitmapCache().GetBitmap(filename, maskname);
}

void GOGUIPanel::SetView(GOGUIPanelView *view) { m_view = view; }

void GOGUIPanel::Init(
  GOConfigReader &cfg,
  GOGUIDisplayMetrics *metrics,
  wxString name,
  wxString group,
  wxString group_name) {
  BasicLoad(cfg, group, false);
  m_OrganController->RegisterSaveableObject(this);
  m_metrics = metrics;
  m_layout = new GOGUILayoutEngine(*m_metrics);
  m_Name = name;
  m_GroupName = group_name;
  m_controls.resize(0);
}

const wxString WX_S_03D = wxT("%s%03d");

wxString compose_group_name(const wxString &groupPrefix, unsigned groupIndex) {
  return wxString::Format(WX_S_03D, groupPrefix, groupIndex);
}

void GOGUIPanel::LoadButton(
  GOConfigReader &cfg,
  GOButtonControl *pButtonControl,
  const wxString &groupPrefix,
  unsigned groupIndex,
  bool isPiston) {
  if (pButtonControl && pButtonControl->IsDisplayed())
    LoadControl(
      new GOGUIButton(this, pButtonControl, isPiston),
      cfg,
      compose_group_name(groupPrefix, groupIndex));
}

void GOGUIPanel::LoadManualButton(
  GOConfigReader &cfg,
  GOButtonControl *pButtonControl,
  const wxString &manualGroup,
  const wxString &groupPrefix,
  unsigned elementIndex,
  bool isPiston) {
  LoadButton(
    cfg,
    pButtonControl,
    groupPrefix,
    cfg.ReadInteger(
      ODFSetting,
      manualGroup,
      compose_group_name(groupPrefix, elementIndex),
      1,
      999),
    isPiston);
}

void GOGUIPanel::Load(GOConfigReader &cfg, const wxString &group) {
  wxString cfgGroup = group;
  wxString panel_group;
  wxString panel_prefix;
  bool is_main_panel;
  bool new_format = false;

  if (
    cfg.ReadInteger(
      ODFSetting,
      wxT("Panel000"),
      wxT("NumberOfGUIElements"),
      0,
      999,
      false,
      -1)
    >= 0)
    new_format = true;

  if (cfgGroup.IsEmpty()) {
    m_Name = m_OrganController->GetChurchName();
    m_GroupName = wxT("");
    cfgGroup = wxT("Organ");
    if (new_format) {
      panel_group = wxT("Panel000");
      panel_prefix = panel_group;
      m_Name
        = cfg.ReadString(ODFSetting, panel_group, wxT("Name"), false, m_Name);
    } else {
      panel_group = cfgGroup;
      panel_prefix = wxEmptyString;
    }
    is_main_panel = true;
  } else {
    panel_group = cfgGroup;
    panel_prefix = panel_group;
    is_main_panel = false;
    m_Name = cfg.ReadStringNotEmpty(ODFSetting, panel_group, wxT("Name"));
    m_GroupName = cfg.ReadString(ODFSetting, panel_group, wxT("Group"), false);
  }
  BasicLoad(cfg, cfgGroup, is_main_panel);
  m_OrganController->RegisterSaveableObject(this);

  m_metrics = new GOGUIHW1DisplayMetrics(cfg, panel_group);
  m_layout = new GOGUILayoutEngine(*m_metrics);

  unsigned first_manual
    = cfg.ReadBoolean(ODFSetting, panel_group, wxT("HasPedals")) ? 0 : 1;
  for (unsigned i = 0; i < first_manual; i++)
    m_layout->RegisterManual(0);

  LoadBackgroundControl(
    new GOGUIHW1Background(this), cfg, wxString::Format(wxT("---")));

  unsigned NumberOfImages = cfg.ReadInteger(
    ODFSetting, panel_group, wxT("NumberOfImages"), 0, 999, false, 0);
  for (unsigned i = 0; i < NumberOfImages; i++) {
    LoadControl(
      new GOGUIImage(this),
      cfg,
      panel_prefix + wxString::Format(wxT("Image%03d"), i + 1));
  }

  if (is_main_panel) {
    for (unsigned i = 0; i < m_OrganController->GetEnclosureCount(); i++)
      if (m_OrganController->GetEnclosureElement(i)->IsDisplayed(new_format)) {
        LoadControl(
          new GOGUIEnclosure(this, m_OrganController->GetEnclosureElement(i)),
          cfg,
          wxString::Format(wxT("Enclosure%03d"), i + 1));
      }

    if (!new_format) {
      unsigned NumberOfSetterElements = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        wxT("NumberOfSetterElements"),
        0,
        999,
        false,
        0);
      for (unsigned i = 0; i < NumberOfSetterElements; i++) {
        wxString buffer = wxString::Format(wxT("SetterElement%03d"), i + 1);
        GOGUIControl *control = CreateGUIElement(cfg, buffer);
        if (!control)
          throw(wxString) wxString::Format(
            _("Unknown SetterElement in section %s"), buffer.c_str());
        LoadControl(control, cfg, buffer);
      }
    }

    for (unsigned i = 0; i < m_OrganController->GetTremulantCount(); i++)
      LoadButton(
        cfg, m_OrganController->GetTremulant(i), wxT("Tremulant"), i + 1);
    for (unsigned i = 0; i < m_OrganController->GetDivisionalCouplerCount();
         i++)
      LoadButton(
        cfg,
        m_OrganController->GetDivisionalCoupler(i),
        wxT("DivisionalCoupler"),
        i + 1);
    for (unsigned i = 0; i < m_OrganController->GetGeneralCount(); i++)
      LoadButton(
        cfg, m_OrganController->GetGeneral(i), wxT("General"), i + 1, true);
    for (unsigned i = 0; i < m_OrganController->GetNumberOfReversiblePistons();
         i++)
      LoadButton(
        cfg,
        m_OrganController->GetPiston(i),
        wxT("ReversiblePiston"),
        i + 1,
        true);
    for (unsigned i = 0; i < m_OrganController->GetSwitchCount(); i++)
      LoadButton(cfg, m_OrganController->GetSwitch(i), wxT("Switch"), i + 1);

    for (unsigned int i = m_OrganController->GetFirstManualIndex();
         i <= m_OrganController->GetManualAndPedalCount();
         i++) {
      GOManual *pManual = m_OrganController->GetManual(i);
      wxString manualGroup = wxString::Format(wxT("Manual%03d"), i);

      if (pManual->IsDisplayed()) {
        LoadBackgroundControl(
          new GOGUIManualBackground(this, m_layout->GetManualNumber()),
          cfg,
          manualGroup);
        LoadControl(
          new GOGUIManual(this, pManual, m_layout->GetManualNumber()),
          cfg,
          manualGroup);
      }
      for (unsigned j = 0; j < pManual->GetODFCouplerCount(); j++)
        LoadManualButton(
          cfg, pManual->GetCoupler(j), manualGroup, wxT("Coupler"), j + 1);
      for (unsigned j = 0; j < pManual->GetStopCount(); j++)
        LoadManualButton(
          cfg, pManual->GetStop(j), manualGroup, wxT("Stop"), j + 1);
      for (unsigned j = 0; j < pManual->GetDivisionalCount(); j++)
        LoadManualButton(
          cfg,
          pManual->GetDivisional(j),
          manualGroup,
          wxT("Divisional"),
          j + 1,
          true);
    }
  } else if (!new_format) {
    unsigned NumberOfEnclosures = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfEnclosures"),
      0,
      m_OrganController->GetEnclosureCount());
    for (unsigned i = 0; i < NumberOfEnclosures; i++) {
      wxString buffer = wxString::Format(wxT("Enclosure%03d"), i + 1);
      unsigned enclosure_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetEnclosureCount());
      buffer = wxString::Format(wxT("Enclosure%03d"), enclosure_nb);
      LoadControl(
        new GOGUIEnclosure(
          this, m_OrganController->GetEnclosureElement(enclosure_nb - 1)),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfSetterElements = cfg.ReadInteger(
      ODFSetting, panel_group, wxT("NumberOfSetterElements"), 0, 999, false);
    for (unsigned i = 0; i < NumberOfSetterElements; i++) {
      wxString buffer = wxString::Format(wxT("SetterElement%03d"), i + 1);
      GOGUIControl *control = CreateGUIElement(cfg, panel_prefix + buffer);
      if (!control)
        throw(wxString) wxString::Format(
          _("Unknown SetterElement in section %s"),
          (panel_prefix + buffer).c_str());
      LoadControl(control, cfg, panel_prefix + buffer);
    }

    unsigned NumberOfTremulants = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfTremulants"),
      0,
      m_OrganController->GetTremulantCount());
    for (unsigned i = 0; i < NumberOfTremulants; i++) {
      wxString buffer = wxString::Format(wxT("Tremulant%03d"), i + 1);
      unsigned tremulant_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetTremulantCount());
      buffer = wxString::Format(wxT("Tremulant%03d"), tremulant_nb);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetTremulant(tremulant_nb - 1)),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfDivisionalCouplers = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfDivisionalCouplers"),
      0,
      m_OrganController->GetDivisionalCouplerCount());
    for (unsigned i = 0; i < NumberOfDivisionalCouplers; i++) {
      wxString buffer = wxString::Format(wxT("DivisionalCoupler%03d"), i + 1);
      unsigned coupler_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetDivisionalCouplerCount());
      buffer = wxString::Format(wxT("DivisionalCoupler%03d"), coupler_nb);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetDivisionalCoupler(coupler_nb - 1)),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfGenerals = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfGenerals"),
      0,
      m_OrganController->GetGeneralCount());
    for (unsigned i = 0; i < NumberOfGenerals; i++) {
      wxString buffer = wxString::Format(wxT("General%03d"), i + 1);
      unsigned general_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetGeneralCount());
      buffer = wxString::Format(wxT("General%03d"), general_nb);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetGeneral(general_nb - 1), true),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfReversiblePistons = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfReversiblePistons"),
      0,
      m_OrganController->GetNumberOfReversiblePistons());
    for (unsigned i = 0; i < NumberOfReversiblePistons; i++) {
      wxString buffer = wxString::Format(wxT("ReversiblePiston%03d"), i + 1);
      unsigned piston_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetNumberOfReversiblePistons());
      buffer = wxString::Format(wxT("ReversiblePiston%03d"), piston_nb);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetPiston(piston_nb - 1), true),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfSwitches = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfSwitches"),
      0,
      m_OrganController->GetSwitchCount(),
      false,
      0);
    for (unsigned i = 0; i < NumberOfSwitches; i++) {
      wxString buffer = wxString::Format(wxT("Switch%03d"), i + 1);
      unsigned switch_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetSwitchCount());
      buffer = wxString::Format(wxT("Switch%03d"), switch_nb);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetSwitch(switch_nb - 1), false),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned nb_manuals = cfg.ReadInteger(
      ODFSetting,
      panel_group,
      wxT("NumberOfManuals"),
      0,
      m_OrganController->GetManualAndPedalCount());
    for (unsigned int i = first_manual; i <= nb_manuals; i++) {
      wxString buffer = wxString::Format(wxT("Manual%03d"), i);
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer = wxString::Format(wxT("Manual%03d"), manual_nb);
      LoadBackgroundControl(
        new GOGUIManualBackground(this, i), cfg, panel_prefix + buffer);
      LoadControl(
        new GOGUIManual(
          this,
          m_OrganController->GetManual(manual_nb),
          m_layout->GetManualNumber()),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(panel_prefix + buffer);
    }

    unsigned NumberOfCouplers = cfg.ReadInteger(
      ODFSetting, panel_group, wxT("NumberOfCouplers"), 0, 999);
    for (unsigned j = 0; j < NumberOfCouplers; j++) {
      wxString buffer = wxString::Format(wxT("Coupler%03dManual"), j + 1);
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer = wxString::Format(wxT("Coupler%03d"), j + 1);
      unsigned coupler_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetManual(manual_nb)->GetODFCouplerCount());
      buffer = wxString::Format(wxT("Coupler%03d"), j + 1);
      LoadControl(
        new GOGUIButton(
          this,
          m_OrganController->GetManual(manual_nb)->GetCoupler(coupler_nb - 1)),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(
        panel_prefix
        + wxString::Format(
          wxT("Manual%03dCoupler%03d"), manual_nb, coupler_nb));
    }

    unsigned NumberOfStops
      = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfStops"), 0, 999);
    for (unsigned j = 0; j < NumberOfStops; j++) {
      wxString buffer = wxString::Format(wxT("Stop%03dManual"), j + 1);
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer = wxString::Format(wxT("Stop%03d"), j + 1);
      unsigned stop_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetManual(manual_nb)->GetStopCount());
      buffer = wxString::Format(wxT("Stop%03d"), j + 1);
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetManual(manual_nb)->GetStop(stop_nb - 1)),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(
        panel_prefix
        + wxString::Format(wxT("Manual%03dStop%03d"), manual_nb, stop_nb));
    }

    unsigned NumberOfDivisionals = cfg.ReadInteger(
      ODFSetting, panel_group, wxT("NumberOfDivisionals"), 0, 999);
    for (unsigned j = 0; j < NumberOfDivisionals; j++) {
      wxString buffer = wxString::Format(wxT("Divisional%03dManual"), j + 1);
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      buffer = wxString::Format(wxT("Divisional%03d"), j + 1);
      unsigned divisional_nb = cfg.ReadInteger(
        ODFSetting,
        panel_group,
        buffer,
        1,
        m_OrganController->GetManual(manual_nb)->GetDivisionalCount());
      buffer = wxString::Format(wxT("Divisional%03d"), j + 1);
      LoadControl(
        new GOGUIButton(
          this,
          m_OrganController->GetManual(manual_nb)->GetDivisional(
            divisional_nb - 1),
          true),
        cfg,
        panel_prefix + buffer);
      cfg.MarkGroupInUse(
        panel_prefix
        + wxString::Format(
          wxT("Manual%03dDivisional%03d"), manual_nb, divisional_nb));
    }
  }

  if (!new_format) {
    unsigned NumberOfLabels
      = cfg.ReadInteger(ODFSetting, panel_group, wxT("NumberOfLabels"), 0, 999);
    for (unsigned i = 0; i < NumberOfLabels; i++) {
      wxString buffer = wxString::Format(wxT("Label%03d"), i + 1);
      LoadControl(new GOGUILabel(this, NULL), cfg, panel_prefix + buffer);
    }
  }

  unsigned NumberOfGUIElements = 0;
  if (new_format)
    NumberOfGUIElements = cfg.ReadInteger(
      ODFSetting, panel_group, wxT("NumberOfGUIElements"), 0, 999, false, 0);
  for (unsigned i = 0; i < NumberOfGUIElements; i++) {
    wxString buffer = panel_group + wxString::Format(wxT("Element%03d"), i + 1);
    wxString type = cfg.ReadString(ODFSetting, buffer, wxT("Type"));
    if (type == wxT("Divisional")) {
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Manual"),
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      unsigned divisional_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Divisional"),
        1,
        m_OrganController->GetManual(manual_nb)->GetDivisionalCount());
      LoadControl(
        new GOGUIButton(
          this,
          m_OrganController->GetManual(manual_nb)->GetDivisional(
            divisional_nb - 1),
          true),
        cfg,
        buffer);
    } else if (type == wxT("Enclosure")) {
      unsigned enclosure_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Enclosure"),
        1,
        m_OrganController->GetEnclosureCount());
      LoadControl(
        new GOGUIEnclosure(
          this, m_OrganController->GetEnclosureElement(enclosure_nb - 1)),
        cfg,
        buffer);
    } else if (type == wxT("Tremulant")) {
      unsigned tremulant_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Tremulant"),
        1,
        m_OrganController->GetTremulantCount());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetTremulant(tremulant_nb - 1)),
        cfg,
        buffer);
    } else if (type == wxT("DivisionalCoupler")) {
      unsigned coupler_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("DivisionalCoupler"),
        1,
        m_OrganController->GetDivisionalCouplerCount());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetDivisionalCoupler(coupler_nb - 1)),
        cfg,
        buffer);
    } else if (type == wxT("General")) {
      unsigned general_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("General"),
        1,
        m_OrganController->GetGeneralCount());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetGeneral(general_nb - 1), true),
        cfg,
        buffer);
    } else if (type == wxT("ReversiblePiston")) {
      unsigned piston_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("ReversiblePiston"),
        1,
        m_OrganController->GetNumberOfReversiblePistons());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetPiston(piston_nb - 1), true),
        cfg,
        buffer);
    } else if (type == wxT("Switch")) {
      unsigned switch_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Switch"),
        1,
        m_OrganController->GetSwitchCount());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetSwitch(switch_nb - 1), false),
        cfg,
        buffer);
    } else if (type == wxT("Coupler")) {
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Manual"),
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      unsigned coupler_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Coupler"),
        1,
        m_OrganController->GetManual(manual_nb)->GetODFCouplerCount());
      LoadControl(
        new GOGUIButton(
          this,
          m_OrganController->GetManual(manual_nb)->GetCoupler(coupler_nb - 1)),
        cfg,
        buffer);
    } else if (type == wxT("Stop")) {
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Manual"),
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      unsigned stop_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Stop"),
        1,
        m_OrganController->GetManual(manual_nb)->GetStopCount());
      LoadControl(
        new GOGUIButton(
          this, m_OrganController->GetManual(manual_nb)->GetStop(stop_nb - 1)),
        cfg,
        buffer);
    } else if (type == wxT("Label")) {
      LoadControl(new GOGUILabel(this, NULL), cfg, buffer);
    } else if (type == wxT("Manual")) {
      unsigned manual_nb = cfg.ReadInteger(
        ODFSetting,
        buffer,
        wxT("Manual"),
        m_OrganController->GetFirstManualIndex(),
        m_OrganController->GetManualAndPedalCount());
      LoadBackgroundControl(
        new GOGUIManualBackground(this, m_layout->GetManualNumber()),
        cfg,
        buffer);
      LoadControl(
        new GOGUIManual(
          this,
          m_OrganController->GetManual(manual_nb),
          m_layout->GetManualNumber()),
        cfg,
        buffer);
    } else {
      GOGUIControl *control = CreateGUIElement(cfg, buffer);
      if (!control)
        throw(wxString) wxString::Format(
          _("Unknown SetterElement in section %s"), buffer.c_str());
      LoadControl(control, cfg, buffer);
    }
  }
}

GOGUIControl *GOGUIPanel::CreateGUIElement(
  GOConfigReader &cfg, wxString group) {
  wxString type = cfg.ReadString(ODFSetting, group, wxT("Type"), true);

  GOButtonControl *button = m_OrganController->GetButtonControl(type, true);
  if (button)
    return new GOGUIButton(this, button, button->IsPiston());

  GOLabelControl *label = m_OrganController->GetLabel(type, true);
  if (label)
    return new GOGUILabel(this, label);

  GOEnclosure *enclosure = m_OrganController->GetEnclosure(type, true);
  if (enclosure)
    return new GOGUIEnclosure(this, enclosure);

  return NULL;
}

void GOGUIPanel::Layout() {
  m_layout->Update();

  for (unsigned i = 0; i < m_controls.size(); i++)
    m_controls[i]->Layout();
}

unsigned GOGUIPanel::GetWidth() { return m_metrics->GetScreenWidth(); }

unsigned GOGUIPanel::GetHeight() { return m_metrics->GetScreenHeight(); }

void GOGUIPanel::SetInitialOpenWindow(bool open) { m_InitialOpenWindow = open; }

void GOGUIPanel::AddEvent(GOGUIControl *control) {
  if (m_view)
    m_view->AddEvent(control);
}

void GOGUIPanel::CaptureSizeInfo(const wxTopLevelWindow &win) {
  GOSizeKeeper::CaptureSizeInfo(win);
  m_InitialOpenWindow = win.IsVisible();
}

void GOGUIPanel::LoadControl(
  GOGUIControl *control, GOConfigReader &cfg, wxString group) {
  control->Load(cfg, group);
  AddControl(control);
}

void GOGUIPanel::LoadBackgroundControl(
  GOGUIControl *control, GOConfigReader &cfg, wxString group) {
  control->Load(cfg, group);
  m_controls.insert(m_BackgroundControls, control);
  m_BackgroundControls++;
}

void GOGUIPanel::AddControl(GOGUIControl *control) {
  m_controls.push_back(control);
}

GOGUIDisplayMetrics *GOGUIPanel::GetDisplayMetrics() { return m_metrics; }

GOGUILayoutEngine *GOGUIPanel::GetLayoutEngine() { return m_layout; }

void GOGUIPanel::PrepareDraw(double scale, GOBitmap *background) {
  for (unsigned i = 0; i < m_controls.size(); i++)
    m_controls[i]->PrepareDraw(scale, background);
}

void GOGUIPanel::Draw(GODC &dc) {
  for (unsigned i = 0; i < m_controls.size(); i++)
    m_controls[i]->Draw(dc);
}

void GOGUIPanel::BasicLoad(
  GOConfigReader &cfg, const wxString &group, bool isOpenByDefault) {
  GOSizeKeeper::Load(cfg, group);
  m_InitialOpenWindow = cfg.ReadBoolean(
    CMBSetting, m_group, wxT("WindowDisplayed"), false, isOpenByDefault);
}

void GOGUIPanel::Save(GOConfigWriter &cfg) {
  GOSizeKeeper::Save(cfg);
  cfg.WriteBoolean(m_group, wxT("WindowDisplayed"), m_InitialOpenWindow);
}

void GOGUIPanel::Modified() { m_OrganController->SetOrganModified(); }

void GOGUIPanel::HandleKey(int key) {
  switch (key) {
  case 259: /* Shift not down */
    m_OrganController->GetSetter()->SetterActive(false);
    break;

  case 260: /* Shift down */
    m_OrganController->GetSetter()->SetterActive(true);
    break;
  }

  m_OrganController->HandleKey(key);
}

void GOGUIPanel::SendMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  for (unsigned i = 0; i < m_controls.size(); i++)
    if (m_controls[i]->HandleMousePress(x, y, right, state))
      return;
}

void GOGUIPanel::HandleMousePress(int x, int y, bool right) {
  GOGUIMouseState tmp;
  GOGUIMouseState &state = right ? tmp : m_MouseState;
  SendMousePress(x, y, right, state);
}

void GOGUIPanel::HandleMouseRelease(bool right) {
  if (!right)
    m_MouseState.clear();
}

void GOGUIPanel::HandleMouseScroll(int x, int y, int amount) {
  for (unsigned i = 0; i < m_controls.size(); i++)
    if (m_controls[i]->HandleMouseScroll(x, y, amount))
      return;
}

const GOBitmap &GOGUIPanel::GetWood(unsigned index) {
  return m_WoodImages[index - 1];
}
