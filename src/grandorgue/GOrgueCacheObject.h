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

#ifndef GORGUECACHEOBJECT_H
#define GORGUECACHEOBJECT_H

#include <wx/string.h>
#include "contrib/sha1.h"

class GOrgueCache;
class GOrgueCacheWriter;

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
	virtual void UpdateHash(SHA_CTX& ctx) = 0;
	virtual const wxString& GetLoadTitle() = 0;
};

#endif
