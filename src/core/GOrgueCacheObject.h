/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECACHEOBJECT_H
#define GORGUECACHEOBJECT_H

#include <wx/string.h>

class GOrgueCache;
class GOrgueCacheWriter;
class GOrgueHash;

class GOrgueCacheObject
{
public:
	virtual ~GOrgueCacheObject()
	{
	}

	virtual void Initialize() = 0;
	virtual void LoadData() = 0;
	virtual bool LoadCache(GOrgueCache& cache) = 0;
	virtual bool SaveCache(GOrgueCacheWriter& cache) = 0;
	virtual void UpdateHash(GOrgueHash& hash) = 0;
	virtual const wxString& GetLoadTitle() = 0;
};

#endif
