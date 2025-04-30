/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREGISTEREDORGAN_H
#define GOREGISTEREDORGAN_H

#include "GOOrgan.h"

#include "midi/elements/GOMidiReceiver.h"

class GOConfig;
class GOConfigReader;
class GOConfigWriter;

class GORegisteredOrgan : public GOOrgan {
private:
  GOMidiReceiver m_midi;

public:
  GORegisteredOrgan(const GOConfig &config, const GOOrgan &organ);
  GORegisteredOrgan(
    GOConfig &config, GOConfigReader &cfg, const wxString &group);

  GOMidiReceiver &GetMIDIReceiver() { return m_midi; }
  const GOMidiReceiver &GetMIDIReceiver() const { return m_midi; }

  void Save(GOConfigWriter &cfg, const wxString &group, GOMidiMap &map) const;

  bool Match(const GOMidiEvent &e);
};

#endif /* GOREGISTEREDORGAN_H */
