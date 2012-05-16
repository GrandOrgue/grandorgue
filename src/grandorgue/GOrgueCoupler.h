/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#ifndef GORGUECOUPLER_H
#define GORGUECOUPLER_H

#include <vector>
#include "GOrgueDrawStop.h"

class GOrgueConfigWriter;
class GrandOrgueFile;
class IniFileConfig;
struct IniFileEnumEntry;

class GOrgueCoupler : public GOrgueDrawstop
{
public:
	typedef enum { COUPLER_NORMAL, COUPLER_BASS, COUPLER_MELODY } GOrgueCouplerType;
private:
	static const struct IniFileEnumEntry m_coupler_types[];
	bool m_UnisonOff;
	bool m_CoupleToSubsequentUnisonIntermanualCouplers;
	bool m_CoupleToSubsequentUpwardIntermanualCouplers;
	bool m_CoupleToSubsequentDownwardIntermanualCouplers;
	bool m_CoupleToSubsequentUpwardIntramanualCouplers;
	bool m_CoupleToSubsequentDownwardIntramanualCouplers;
	GOrgueCouplerType m_CouplerType;
	unsigned m_SourceManual;
	unsigned m_DestinationManual;
	int m_DestinationKeyshift;
	int m_Keyshift;
	/* Input state to the coupler */
	std::vector<unsigned> m_KeyState;
	/* Intended output state always assuming the coupler is ON */
	std::vector<unsigned> m_InternalState;
	/* Current ouput state */
	std::vector<unsigned> m_OutState;
	int m_CurrentTone;
	int m_LastTone;
	int m_FirstMidiNote;
	unsigned m_FirstLogicalKey;
	unsigned m_NumberOfKeys;

	void ChangeKey(int note, int on);
	void SetOut(int note, int on);
	unsigned GetInternalState(int note);
public:

	GOrgueCoupler(GrandOrgueFile* organfile, unsigned sourceManual);
	void Load(IniFileConfig& cfg, wxString group, wxString name = wxT(""), bool unison_off = false, bool recursive = false, int keyshift = 0, int dest_manual = 0, GOrgueCouplerType coupler_type = COUPLER_NORMAL);
	void Save(GOrgueConfigWriter& cfg);
	void SetKey(unsigned note, int on, GOrgueCoupler* prev);
	void Set(bool on);
	void PreparePlayback();
	bool IsIntermanual();
	bool IsUnisonOff();

};

#endif
