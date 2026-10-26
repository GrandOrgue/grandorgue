/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDISENDINGOBJECT_H
#define GOMIDISENDINGOBJECT_H

#include "midi/elements/GOMidiSender.h"

#include "GOMidiPlayingObject.h"

class GOOrganModel;

class GOMidiSendingObject : public GOMidiPlayingObject {
private:
  GOMidiSender m_sender;

  /**
   * Whether PreparePlayback(), PrepareRecording() and AbortPlayback() send
   * MIDI events (the name, the current or the empty value). It does not affect
   * the ordinary sends during the work. The base class lifecycle calls (the
   * receiver reset) are always done.
   */
  bool m_IsLifecycleSendingEnabled = true;

protected:
  GOMidiSendingObject(
    GOOrganModel &organModel,
    ObjectType objectType,
    GOMidiSenderType senderType);

  ~GOMidiSendingObject();

  void LoadMidiObject(
    GOConfigReader &cfg, const wxString &group, GOMidiMap &midiMap) override;

  void SendMidiValue(bool value) { m_sender.SetDisplay(value); }
  void SendMidiValue(int value) { m_sender.SetValue(value); }
  void SendMidiValue(const wxString &value) { m_sender.SetLabel(value); }
  void SendMidiKey(unsigned key, unsigned value) {
    m_sender.SetKey(key, value);
  }
  void SendEmptyMidiKey() { m_sender.ResetKey(); }

  /**
   * The midi object should send the current midi value with certain
   * SendMidiValue()
   */
  virtual void SendCurrentMidiValue() {}
  /**
   * The midi object should send the empty midi value with certain
   * SendMidiValue()
   */
  virtual void SendEmptyMidiValue() {}

  /**
   * Send current object state event again. It calls SendCurrentMidiValue()
   */
  void ResendMidi();

  void OnSettingsApplied() override;

public:
  /** @return true if the lifecycle methods send MIDI events */
  bool IsLifecycleSendingEnabled() const { return m_IsLifecycleSendingEnabled; }

  /**
   * Enables or disables the MIDI sending in PreparePlayback(),
   * PrepareRecording() and AbortPlayback().
   * @param isEnabled true: send (the default); false: do not send
   */
  void SetLifecycleSendingEnabled(bool isEnabled) {
    m_IsLifecycleSendingEnabled = isEnabled;
  }

  void PreparePlayback() override;
  void PrepareRecording() override;
  void AbortPlayback() override;

public:
  virtual void SetElementId(int id);
};

#endif /* GOMIDISENDINGOBJECT_H */
