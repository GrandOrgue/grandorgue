/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIControl.h"

#include "GOGUIPanel.h"
#include "GOOrganController.h"
#include "config/GOConfigReader.h"

GOGUIControl::GOGUIControl(GOGUIPanel *panel, GOControl *pControl)
  : p_panel(panel),
    p_control(pControl),
    m_BoundingRect(0, 0, 0, 0),
    m_DrawPending(false) {
  p_metrics = panel->GetDisplayMetrics();
  p_layout = panel->GetLayoutEngine();
  p_panel->GetOrganFile()->RegisterControlChangedHandler(this);
  p_panel->GetOrganFile()->RegisterSaveableObject(this);
}

GOGUIControl::~GOGUIControl() {
  // in the reverse order of the constructor
  p_panel->GetOrganFile()->UnregisterSaveableObject(this);
  p_panel->GetOrganFile()->UnRegisterControlChangedHandler(this);
}

void GOGUIControl::Init(GOConfigReader &cfg, wxString group) {
  m_group = group;
}

void GOGUIControl::Load(GOConfigReader &cfg, wxString group) {
  m_group = group;
}

void GOGUIControl::Layout() {}

void GOGUIControl::Save(GOConfigWriter &cfg) {}

void GOGUIControl::ControlChanged(GOControl *control) {
  if (control == p_control)
    if (!m_DrawPending) {
      m_DrawPending = true;
      p_panel->AddEvent(this);
    }
}

void GOGUIControl::PrepareDraw(double scale, GOBitmap *background) {}

void GOGUIControl::Draw(GODC &dc) { m_DrawPending = false; }

bool GOGUIControl::HandleMousePress(
  int x, int y, bool right, GOGUIMouseState &state) {
  return false;
}

bool GOGUIControl::HandleMouseScroll(int x, int y, int amount) { return false; }

const wxRect &GOGUIControl::GetBoundingRect() { return m_BoundingRect; }
