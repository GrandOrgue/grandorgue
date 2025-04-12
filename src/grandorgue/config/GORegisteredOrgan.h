/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREGISTEREDORGAN_H
#define GOREGISTEREDORGAN_H

#include "GOOrgan.h"

#include "midi/GOMidiReceiverBase.h"

class GOConfigReader;
class GOConfigWriter;
class GOMidiMap;

class GORegisteredOrgan : public GOOrgan {
private:
  GOMidiReceiverBase m_midi;

public:
  GORegisteredOrgan(GOConfigReader &cfg, const wxString &group, GOMidiMap &map);

  GOMidiReceiverBase &GetMIDIReceiver() { return m_midi; }
  const GOMidiReceiverBase &GetMIDIReceiver() const { return m_midi; }

  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const;

  bool Match(const GOMidiEvent &e);
};

#endif /* GOREGISTEREDORGAN_H */
