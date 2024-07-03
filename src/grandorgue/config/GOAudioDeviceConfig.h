/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIODEVICECONFIG_H
#define GOAUDIODEVICECONFIG_H

class GOAudioDeviceConfig {
public:
  struct GroupOutput {
    wxString name;
    float left;
    float right;
  };

  wxString name;
  unsigned channels;
  unsigned desired_latency;
  std::vector<std::vector<GroupOutput>> scale_factors;
};

#endif /* GOAUDIODEVICECONFIG_H */
