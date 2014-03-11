/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
