/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIINPUTMERGER_H
#define GOMIDIINPUTMERGER_H

class GOMidiEvent;

class GOMidiMerger {
private:
  unsigned m_BankLsb[16];
  unsigned m_BankMsb[16];
  bool m_Rpn;
  unsigned m_RpnLsb[16];
  unsigned m_RpnMsb[16];
  unsigned m_NrpnLsb[16];
  unsigned m_NrpnMsb[16];

public:
  GOMidiMerger();

  void Clear();
  bool Process(GOMidiEvent &e);
};

#endif
