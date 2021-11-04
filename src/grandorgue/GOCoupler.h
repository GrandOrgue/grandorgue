/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOCOUPLER_H
#define GOCOUPLER_H

#include "GODrawStop.h"
#include <vector>

class GOConfigReader;
class GOConfigWriter;
class GODefinitionFile;
struct IniFileEnumEntry;

class GOCoupler : public GODrawstop
{
public:
	typedef enum { COUPLER_NORMAL, COUPLER_BASS, COUPLER_MELODY } GOCouplerType;
private:
	static const struct IniFileEnumEntry m_coupler_types[];
	bool m_UnisonOff;
	bool m_CoupleToSubsequentUnisonIntermanualCouplers;
	bool m_CoupleToSubsequentUpwardIntermanualCouplers;
	bool m_CoupleToSubsequentDownwardIntermanualCouplers;
	bool m_CoupleToSubsequentUpwardIntramanualCouplers;
	bool m_CoupleToSubsequentDownwardIntramanualCouplers;
	GOCouplerType m_CouplerType;
	unsigned m_SourceManual;
	unsigned m_CouplerID;
	unsigned m_DestinationManual;
	int m_DestinationKeyshift;
	int m_Keyshift;
	/* Input state to the coupler */
	std::vector<unsigned> m_KeyVelocity;
	/* Intended output state always assuming the coupler is ON */
	std::vector<unsigned> m_InternalVelocity;
	/* Current ouput state */
	std::vector<unsigned> m_OutVelocity;
	int m_CurrentTone;
	int m_LastTone;
	int m_FirstMidiNote;
	unsigned m_FirstLogicalKey;
	unsigned m_NumberOfKeys;

	void ChangeKey(int note, unsigned velocity);
	void SetOut(int note, unsigned velocity);
	unsigned GetInternalState(int note);
	void ChangeState(bool on);
	void SetupCombinationState();

	void PreparePlayback();
	void StartPlayback();

public:

	GOCoupler(GODefinitionFile* organfile, unsigned sourceManual);
	void Init(GOConfigReader& cfg, wxString group, wxString name, bool unison_off, bool recursive, int keyshift, int dest_manual, GOCouplerType coupler_type);
	void Load(GOConfigReader& cfg, wxString group);
	void SetKey(unsigned note, const std::vector<unsigned>& velocities, const std::vector<GOCoupler*>& couplers);
	bool IsIntermanual();
	bool IsUnisonOff();

	wxString GetMidiType();
};

#endif
