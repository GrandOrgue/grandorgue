/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiObjectWithShortcut.h"

GOMidiObjectWithShortcut::GOMidiObjectWithShortcut(
  GOOrganModel &organModel,
  ObjectType objectType,
  GOMidiSenderType senderType,
  GOMidiReceiverType receiverType,
  GOMidiShortcutReceiverType shortcutType)
  : GOMidiReceivingSendingObject(
    organModel, objectType, senderType, receiverType),
    m_ShortcutReceiver(shortcutType) {
  SetMidiShortcutReceiver(&m_ShortcutReceiver);
}

GOMidiObjectWithShortcut::~GOMidiObjectWithShortcut() {
  SetMidiShortcutReceiver(nullptr);
}

void GOMidiObjectWithShortcut::HandleKey(int key) {
  if (!IsReadOnly()) {
    auto matchType = m_ShortcutReceiver.Match(key);

    if (matchType != GOMidiShortcutReceiver::KEY_MATCH_NONE)
      OnShortcutKeyReceived(matchType, key);
  }
}
