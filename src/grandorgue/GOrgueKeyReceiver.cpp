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
 */

#include "GOrgueKeyReceiver.h"
#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"

GOrgueKeyReceiver::GOrgueKeyReceiver() :
	m_ShortcutKey(0)
{
}

void GOrgueKeyReceiver::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_ShortcutKey = cfg.ReadInteger(UserSetting, group, wxT("ShortcutKey"), 0, 255, false, m_ShortcutKey);
}

void GOrgueKeyReceiver::Save(GOrgueConfigWriter& cfg, wxString group)
{
	cfg.Write(group, wxT("ShortcutKey"), (int)m_ShortcutKey);
}

bool GOrgueKeyReceiver::Match(unsigned key)
{
	if (m_ShortcutKey == key)
		return true;
	return false;
}

unsigned GOrgueKeyReceiver::GetShortcut()
{
	return m_ShortcutKey;
}

void GOrgueKeyReceiver::SetShortcut(unsigned key)
{
	m_ShortcutKey = key;
}
