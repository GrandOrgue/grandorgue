/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIRECEIVER_H
#define GORGUEMIDIRECEIVER_H

#include "GOrgueMidiReceiverBase.h"

class GrandOrgueFile;

class GOrgueMidiReceiver : public GOrgueMidiReceiverBase
{
private:
	GrandOrgueFile* m_organfile;
	int m_Index;

protected:
	void Preconfigure(GOrgueConfigReader& cfg, wxString group);
	int GetTranspose();

public:
	GOrgueMidiReceiver(GrandOrgueFile* organfile, MIDI_RECEIVER_TYPE type);

	void Load(GOrgueConfigReader& cfg, wxString group, GOrgueMidiMap& map);

	void SetIndex(int index);

	void Assign(const GOrgueMidiReceiverData& data);
};

#endif
