/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEKEYRECEIVER_H
#define GORGUEKEYRECEIVER_H

#include "GOrgueKeyReceiverData.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GrandOrgueFile;

class GOrgueKeyReceiver : public GOrgueKeyReceiverData {
private:
	GrandOrgueFile* m_organfile;

public:
	GOrgueKeyReceiver(GrandOrgueFile* organfile, KEY_RECEIVER_TYPE type);

	void Load(GOrgueConfigReader& cfg, wxString group);
	void Save(GOrgueConfigWriter& cfg, wxString group);

	KEY_MATCH_TYPE Match(unsigned key);

	void Assign(const GOrgueKeyReceiverData& data);
};

#endif
