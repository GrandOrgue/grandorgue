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

#ifndef GORGUEBITMAPCACHE_H
#define GORGUEBITMAPCACHE_H

#include <wx/wx.h>
#include "ptrvector.h"

class GrandOrgueFile;

class GOrgueBitmapCache
{
private:
	GrandOrgueFile* m_organfile;
	ptr_vector<wxBitmap> m_Bitmaps;
	std::vector<wxString> m_Filenames;
	std::vector<wxString> m_Masknames;

public:
	GOrgueBitmapCache(GrandOrgueFile* organfile);
	virtual ~GOrgueBitmapCache();

	void RegisterBitmap(wxBitmap* bitmap, wxString filename, wxString maskname = wxEmptyString);
	wxBitmap* GetBitmap(wxString filename, wxString maskName = wxEmptyString);
};

#endif
