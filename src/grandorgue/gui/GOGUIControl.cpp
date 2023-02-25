/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOGUIControl.h"

#include "GOGUIPanel.h"
#include "GOOrganController.h"
#include "config/GOConfigReader.h"

GOGUIControl::GOGUIControl(GOGUIPanel *panel, void *control)
  : m_panel(panel),
    m_control(control),
    m_BoundingRect(0, 0, 0, 0),
    m_DrawPending(false) {
  m_metrics = panel->GetDisplayMetrics();
  m_layout = panel->GetLayoutEngine();
  m_panel->GetOrganFile()->RegisterControlChangedHandler(this);
}

GOGUIControl::~GOGUIControl() {}

void GOGUIControl::Init(GOConfigReader &cfg, wxString group) {
  m_panel->GetOrganFile()->RegisterSaveableObject(this);
  m_group = group;
}

void GOGUIControl::Load(GOConfigReader &cfg, wxString group) {
  m_panel->GetOrganFile()->RegisterSaveableObject(this);
  m_group = group;
}

void GOGUIControl::Layout() {}

void GOGUIControl::Save(GOConfigWriter &cfg) {}

void GOGUIControl::ControlChanged(void *control) {
  if (control == m_control)
    if (!m_DrawPending) {
      m_DrawPending = true;
      m_panel->AddEvent(this);
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
