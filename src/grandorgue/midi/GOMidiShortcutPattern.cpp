/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiShortcutPattern.h"

GOKeyReceiverData::GOKeyReceiverData(KEY_RECEIVER_TYPE type)
  : m_type(type), m_ShortcutKey(0), m_MinusKey(0) {}

GOKeyReceiverData::~GOKeyReceiverData() {}

KEY_RECEIVER_TYPE GOKeyReceiverData::GetType() const { return m_type; }

unsigned GOKeyReceiverData::GetShortcut() { return m_ShortcutKey; }

void GOKeyReceiverData::SetShortcut(unsigned key) { m_ShortcutKey = key; }

unsigned GOKeyReceiverData::GetMinusKey() { return m_MinusKey; }

void GOKeyReceiverData::SetMinusKey(unsigned key) { m_MinusKey = key; }
