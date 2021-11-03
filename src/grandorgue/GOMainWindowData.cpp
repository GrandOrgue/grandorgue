/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOMainWindowData.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"
#include "GODefinitionFile.h"

GOMainWindowData::GOMainWindowData(GODefinitionFile* organfile) :
	m_organfile(organfile)
{
}

GOMainWindowData::~GOMainWindowData()
{
}

void GOMainWindowData::Load(GOConfigReader& cfg, wxString group)
{
	m_organfile->RegisterSaveableObject(this);
	int x = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowX"), -20, 10000, false, 0);
	int y = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowY"), -20, 10000, false, 0);
	int w = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowWidth"), 0, 10000, false, 0);
	int h = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowHeight"), 0, 10000, false, 0);
	m_size = wxRect(x, y, w, h);
}

void GOMainWindowData::Save(GOConfigWriter& cfg)
{
	wxRect size = m_size;
	int x = size.GetLeft();
	int y = size.GetTop();
	if (x < -20)
		x = -20;
	if (y < -20)
		y = -20;
	cfg.WriteInteger(m_group, wxT("WindowX"), x);
	cfg.WriteInteger(m_group, wxT("WindowY"), y);
	cfg.WriteInteger(m_group, wxT("WindowWidth"), size.GetWidth());
	cfg.WriteInteger(m_group, wxT("WindowHeight"), size.GetHeight());
}

wxRect GOMainWindowData::GetWindowSize()
{
	return m_size;
}

void GOMainWindowData::SetWindowSize(wxRect rect)
{
	m_size = rect;
}

