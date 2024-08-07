/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIODEVICENODE_H
#define GOAUDIODEVICENODE_H

#include "GODeviceNamePattern.h"

class GOConfigReader;
class GOConfigWriter;

class GOAudioDeviceNode : public GODeviceNamePattern {
private:
  static constexpr unsigned DEFAULT_LATENCY = 50;

  unsigned m_DesiredLatency = DEFAULT_LATENCY;

public:
  unsigned GetDesiredLatency() const { return m_DesiredLatency; }
  void SetDesiredLatenct(unsigned value) { m_DesiredLatency = value; }

  void LoadDeviceNode(
    GOConfigReader &cfg, const wxString &group, const wxString &prefix);
  void SaveDeviceNode(
    GOConfigWriter &cfg, const wxString &group, const wxString &prefix) const;
};

#endif /* GOAUDIODEVICENODE_H */
