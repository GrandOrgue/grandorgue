/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIOUTPORT_H
#define GOMIDIOUTPORT_H

#include <wx/string.h>

#include "GOMidiPort.h"
#include "midi/GOMidiOutputMerger.h"
#include "ptrvector.h"

class GOMidiEvent;

class GOMidiOutPort : public GOMidiPort {
protected:
  GOMidiOutputMerger m_merger;

  virtual void SendData(std::vector<unsigned char> &msg) = 0;

public:
  GOMidiOutPort(
    GOMidi *midi,
    const wxString &portName,
    const wxString &apiName,
    const wxString &deviceName,
    const wxString &fullName);

  virtual ~GOMidiOutPort();

  virtual const wxString GetMyNativePortName() const;

  virtual bool Open(unsigned id);

  void Send(const GOMidiEvent &e);
};

#endif
