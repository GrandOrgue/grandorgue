/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOGUIControl.h"

#include "GOGUIPanel.h"
#include "GOrgueConfigReader.h"
#include "GrandOrgueFile.h"

GOGUIControl::GOGUIControl(GOGUIPanel* panel,void* control) :
	m_panel(panel),
	m_control(control),
	m_BoundingRect(0, 0, 0, 0),
	m_DrawPending(false)
{
	m_metrics = panel->GetDisplayMetrics();
	m_layout = panel->GetLayoutEngine();
	m_panel->GetOrganFile()->RegisterControlChangedHandler(this);
}

GOGUIControl::~GOGUIControl()
{
}

void GOGUIControl::Init(GOrgueConfigReader& cfg, wxString group)
{
	m_panel->GetOrganFile()->RegisterSaveableObject(this);
	m_group = group;
}

void GOGUIControl::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_panel->GetOrganFile()->RegisterSaveableObject(this);
	m_group = group;
}

void GOGUIControl::Layout()
{
}

void GOGUIControl::Save(GOrgueConfigWriter& cfg)
{
}

void GOGUIControl::ControlChanged(void* control)
{
	if (control == m_control)
		if (!m_DrawPending)
		{
			m_DrawPending = true;
			m_panel->AddEvent(this);
		}
}

void GOGUIControl::PrepareDraw(double scale, GOrgueBitmap* background)
{
}

void GOGUIControl::Draw(GOrgueDC& dc)
{
	m_DrawPending = false;
}

bool GOGUIControl::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	return false;
}

bool GOGUIControl::HandleMouseScroll(int x, int y, int amount)
{
	return false;
}

const wxRect& GOGUIControl::GetBoundingRect()
{
	return m_BoundingRect;
}
