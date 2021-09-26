/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEREFERENCEPIPE_H
#define GORGUEREFERENCEPIPE_H

#include "GOrgueCacheObject.h"
#include "GOrguePipe.h"

class GOrgueReferencePipe : public GOrguePipe, private GOrgueCacheObject
{
private:
	GOrguePipe* m_Reference;
	unsigned m_ReferenceID;
	wxString m_Filename;

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(GOrgueHash& hash);
	const wxString& GetLoadTitle();

	void Change(unsigned velocity, unsigned old_velocity);

public:
	GOrgueReferencePipe(GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number);

	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
};

#endif
