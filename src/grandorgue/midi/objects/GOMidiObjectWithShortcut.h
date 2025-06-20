/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOBJECTWITHSHORTCUT_H
#define GOMIDIOBJECTWITHSHORTCUT_H

#include "midi/elements/GOMidiShortcutReceiver.h"

#include "GOMidiReceivingSendingObject.h"

class GOMidiObjectWithShortcut : public GOMidiReceivingSendingObject {
private:
  GOMidiShortcutReceiver m_ShortcutReceiver;

  void HandleKey(int key) override;

protected:
  GOMidiObjectWithShortcut(
    GOOrganModel &organModel,
    const wxString &midiTypeCode,
    const wxString &midiTypeName,
    GOMidiSenderType senderType,
    GOMidiReceiverType receiverType,
    GOMidiShortcutReceiverType shortcutType);

  virtual ~GOMidiObjectWithShortcut();

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;
  void SaveMidiObject(
    GOConfigWriter &cfg,
    const wxString &group,
    GOMidiMap &midiMap) const override;

  virtual void OnShortcutKeyReceived(
    GOMidiShortcutReceiver::MatchType matchType, int key)
    = 0;

public:
  void SetShortcutKey(unsigned key) { m_ShortcutReceiver.SetShortcut(key); }
};

#endif /* GOMIDIOBJECTWITHSHORTCUT_H */
