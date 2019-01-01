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

#include "GOrgueKeyReceiverData.h"

GOrgueKeyReceiverData::GOrgueKeyReceiverData(KEY_RECEIVER_TYPE type) :
	m_type(type),
	m_ShortcutKey(0),
	m_MinusKey(0)
{
}

GOrgueKeyReceiverData::~GOrgueKeyReceiverData()
{
}

KEY_RECEIVER_TYPE GOrgueKeyReceiverData::GetType() const
{
	return m_type;
}

unsigned GOrgueKeyReceiverData::GetShortcut()
{
	return m_ShortcutKey;
}

void GOrgueKeyReceiverData::SetShortcut(unsigned key)
{
	m_ShortcutKey = key;
}

unsigned GOrgueKeyReceiverData::GetMinusKey()
{
	return m_MinusKey;
}

void GOrgueKeyReceiverData::SetMinusKey(unsigned key)
{
	m_MinusKey = key;
}
