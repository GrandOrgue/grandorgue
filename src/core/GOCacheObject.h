/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOCACHEOBJECT_H
#define GOCACHEOBJECT_H

#include <wx/string.h>

class GOCache;
class GOCacheWriter;
class GOHash;

class GOCacheObject
{
public:
	virtual ~GOCacheObject()
	{
	}

	virtual void Initialize() = 0;
	virtual void LoadData() = 0;
	virtual bool LoadCache(GOCache& cache) = 0;
	virtual bool SaveCache(GOCacheWriter& cache) = 0;
	virtual void UpdateHash(GOHash& hash) = 0;
	virtual const wxString& GetLoadTitle() = 0;
};

#endif
