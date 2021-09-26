/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIMERGER_H
#define GORGUEMIDIMERGER_H

class GOrgueMidiEvent;

class GOrgueMidiMerger
{
private:
	unsigned m_BankLsb[16];
	unsigned m_BankMsb[16];
	bool m_Rpn;
	unsigned m_RpnLsb[16];
	unsigned m_RpnMsb[16];
	unsigned m_NrpnLsb[16];
	unsigned m_NrpnMsb[16];

public:
	GOrgueMidiMerger();

	void Clear();
	bool Process(GOrgueMidiEvent& e);
};

#endif
