/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOrgueMainWindowData.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GrandOrgueFile.h"

GOrgueMainWindowData::GOrgueMainWindowData(GrandOrgueFile* organfile) :
	m_organfile(organfile)
{
}

GOrgueMainWindowData::~GOrgueMainWindowData()
{
}

void GOrgueMainWindowData::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_organfile->RegisterSaveableObject(this);
	int x = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowX"), -20, 10000, false, 0);
	int y = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowY"), -20, 10000, false, 0);
	int w = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowWidth"), 0, 10000, false, 0);
	int h = cfg.ReadInteger(CMBSetting, m_group, wxT("WindowHeight"), 0, 10000, false, 0);
	m_size = wxRect(x, y, w, h);
}

void GOrgueMainWindowData::Save(GOrgueConfigWriter& cfg)
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

wxRect GOrgueMainWindowData::GetWindowSize()
{
	return m_size;
}

void GOrgueMainWindowData::SetWindowSize(wxRect rect)
{
	m_size = rect;
}

