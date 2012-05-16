/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#include "GOGUIImage.h"
#include "GOGUIDisplayMetrics.h"
#include "GOGUIPanel.h"
#include "IniFileConfig.h"

GOGUIImage::GOGUIImage(GOGUIPanel* panel) :
	GOGUIControl(panel, NULL),
	m_TileOffsetX(0),
	m_TileOffsetY(0)
{
}

void GOGUIImage::Load(IniFileConfig& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
	int x, y, w, h;
	wxString image_mask_file;
	wxString image_file;

	image_file = cfg.ReadString(ODFSetting, group, wxT("Image"), 255, true);
	image_mask_file = cfg.ReadString(ODFSetting, group, wxT("Mask"), 255, false, wxEmptyString);

	m_Bitmap = m_panel->LoadBitmap(image_file, image_mask_file);

	x = cfg.ReadInteger(ODFSetting, group, wxT("PositionX"), 0, m_metrics->GetScreenWidth(), false, 0);
	y = cfg.ReadInteger(ODFSetting, group, wxT("PositionY"), 0, m_metrics->GetScreenHeight(), false, 0);
	w = cfg.ReadInteger(ODFSetting, group, wxT("Width"), 1, m_metrics->GetScreenWidth(), false, m_Bitmap->GetWidth());
	h = cfg.ReadInteger(ODFSetting, group, wxT("Height"), 1, m_metrics->GetScreenHeight(), false, m_Bitmap->GetHeight());
	m_BoundingRect = wxRect(x, y, w, h);

	m_TileOffsetX = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetX"), 0, m_Bitmap->GetWidth() - 1, false, 0);
	m_TileOffsetY = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetY"), 0, m_Bitmap->GetHeight() - 1, false, 0);
}

void GOGUIImage::Draw(wxDC* dc)
{
	m_panel->TileBitmap(dc, m_Bitmap, m_BoundingRect, m_TileOffsetX, m_TileOffsetY);
	GOGUIControl::Draw(dc);
}
