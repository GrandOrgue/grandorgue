/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueKeyReceiver.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GrandOrgueFile.h"

GOrgueKeyReceiver::GOrgueKeyReceiver(GrandOrgueFile* organfile, KEY_RECEIVER_TYPE type) :
	GOrgueKeyReceiverData(type),
	m_organfile(organfile)
{
}

void GOrgueKeyReceiver::Load(GOrgueConfigReader& cfg, wxString group)
{
	if (m_type == KEY_RECV_ENCLOSURE)
	{
		m_ShortcutKey = cfg.ReadInteger(CMBSetting, group, wxT("PlusKey"), 0, 255, false, 0);
		m_MinusKey = cfg.ReadInteger(CMBSetting, group, wxT("MinusKey"), 0, 255, false, 0);
	}
	else
	{
		m_ShortcutKey = cfg.ReadInteger(ODFSetting, group, wxT("ShortcutKey"), 0, 255, false, m_ShortcutKey);
		m_ShortcutKey = cfg.ReadInteger(CMBSetting, group, wxT("ShortcutKey"), 0, 255, false, m_ShortcutKey);
	}
}

void GOrgueKeyReceiver::Save(GOrgueConfigWriter& cfg, wxString group)
{
	if (m_type == KEY_RECV_ENCLOSURE)
	{
		cfg.WriteInteger(group, wxT("PlusKey"), m_ShortcutKey);
		cfg.WriteInteger(group, wxT("MinusKey"), m_MinusKey);
	}
	else
	{
		cfg.WriteInteger(group, wxT("ShortcutKey"), m_ShortcutKey);
	}
}

KEY_MATCH_TYPE GOrgueKeyReceiver::Match(unsigned key)
{
	if (m_ShortcutKey == key)
		return KEY_MATCH;
	if (m_MinusKey == key)
		return KEY_MATCH_MINUS;
	return KEY_MATCH_NONE;
}

void GOrgueKeyReceiver::Assign(const GOrgueKeyReceiverData& data)
{
	*(GOrgueKeyReceiverData*)this = data;
	if (m_organfile)
		m_organfile->Modified();
}
