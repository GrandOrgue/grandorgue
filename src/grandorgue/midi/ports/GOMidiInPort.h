/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIINPORT_H
#define GOMIDIINPORT_H

#include "GOMidiPort.h"
#include "midi/GOMidiInputMerger.h"
#include "ptrvector.h"

class GOMidiInPort : public GOMidiPort {
protected:
  GOMidiMerger m_merger;
  int m_ChannelShift;

  virtual const wxString GetMyNativePortName() const;

  void Receive(const std::vector<unsigned char> msg);

public:
  GOMidiInPort(
    GOMidi *midi,
    const wxString &portName,
    const wxString &apiName,
    const wxString &deviceName,
    const wxString &fullName);

  virtual ~GOMidiInPort();

  virtual bool Open(unsigned id, int channel_shift);
  bool Open(unsigned id) { return Open(id, 0); }
};

#endif
