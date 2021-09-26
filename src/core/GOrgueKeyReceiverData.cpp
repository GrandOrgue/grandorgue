/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
