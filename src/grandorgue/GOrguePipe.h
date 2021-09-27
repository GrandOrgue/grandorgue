/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include "GOrguePlaybackStateHandler.h"
#include <vector>
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueRank;
class GOrgueTemperament;
class GrandOrgueFile;

class GOrguePipe : private GOrguePlaybackStateHandler
{
private:
	unsigned m_Velocity;
	std::vector<unsigned> m_Velocities;

protected:
	GrandOrgueFile* m_organfile;
	GOrgueRank* m_Rank;
	unsigned m_MidiKeyNumber;

	virtual void Change(unsigned velocity, unsigned old_velocity) = 0;

	void AbortPlayback();
	void StartPlayback();
	void PreparePlayback();
	void PrepareRecording();

public:
	GOrguePipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number);
	virtual ~GOrguePipe();
	virtual void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix) = 0;
	void Set(unsigned velocity, unsigned referenceID = 0);
	unsigned RegisterReference(GOrguePipe* pipe);
	virtual void SetTemperament(const GOrgueTemperament& temperament);
};

#endif
