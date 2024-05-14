/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSWITCH_H
#define GOSWITCH_H

#include "GODrawStop.h"

class GOSwitch : public GODrawstop {
  // the number of manual for mainual switches
  // -2 global switches not referenced from manuals
  // -1 global switches referenced from more than one manual

  int m_AssociatedManualN = -2;
  unsigned m_IndexInManual = 0;

protected:
  void ChangeState(bool) override {}

public:
  GOSwitch(GOOrganModel &organModel) : GODrawstop(organModel) {}

  // Return -1 for all kinds of global switches
  int GetAssociatedManualN() const { return std::max(m_AssociatedManualN, -1); }
  unsigned GetIndexInManual() const { return m_IndexInManual; }

  // Set m_AssociatedManualN.
  // Check that the switch is referenced not more than once
  void AssociateWithManual(int manualN, unsigned indexInManual);

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif
