/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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

#include <wx/filename.h>
#include "Images.h"
#include "GOrgueBitmapCache.h"
#include "GrandOrgueFile.h"


GOrgueBitmapCache::GOrgueBitmapCache(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Bitmaps(),
	m_Filenames(),
	m_Masknames()
{
	RegisterBitmap(new wxBitmap(GetImage_draw1off()), wxT("GO:drawstop01_off"));
	RegisterBitmap(new wxBitmap(GetImage_draw1on()), wxT("GO:drawstop01_on"));
	RegisterBitmap(new wxBitmap(GetImage_draw2off()), wxT("GO:drawstop02_off"));
	RegisterBitmap(new wxBitmap(GetImage_draw2on()), wxT("GO:drawstop02_on"));

	RegisterBitmap(new wxBitmap(GetImage_hpiston()), wxT("GO:piston01_off"));
	RegisterBitmap(new wxBitmap(GetImage_hpiston2()), wxT("GO:piston01_on"));
	RegisterBitmap(new wxBitmap(GetImage_fpiston()), wxT("GO:piston02_off"));
	RegisterBitmap(new wxBitmap(GetImage_fpiston2()), wxT("GO:piston02_on"));

	RegisterBitmap(new wxBitmap(GetImage_label()), wxT("GO:label01"));

	RegisterBitmap(new wxBitmap(GetImage_Enclosure0()), wxT("GO:enclosure0"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure1()), wxT("GO:enclosure1"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure2()), wxT("GO:enclosure2"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure3()), wxT("GO:enclosure3"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure4()), wxT("GO:enclosure4"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure5()), wxT("GO:enclosure5"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure6()), wxT("GO:enclosure6"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure7()), wxT("GO:enclosure7"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure8()), wxT("GO:enclosure8"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure9()), wxT("GO:enclosure9"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure10()), wxT("GO:enclosure10"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure11()), wxT("GO:enclosure11"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure12()), wxT("GO:enclosure12"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure13()), wxT("GO:enclosure13"));
	RegisterBitmap(new wxBitmap(GetImage_Enclosure14()), wxT("GO:enclosure14"));
}

GOrgueBitmapCache::~GOrgueBitmapCache()
{
}

void GOrgueBitmapCache::RegisterBitmap(wxBitmap* bitmap, wxString filename, wxString maskname)
{
	m_Bitmaps.push_back(bitmap);
	m_Filenames.push_back(filename);
	m_Masknames.push_back(maskname);
}

wxBitmap* GOrgueBitmapCache::GetBitmap(wxString filename, wxString maskName)
{
	for(unsigned i = 0; i < m_Filenames.size(); i++)
		if (m_Filenames[i] == filename && m_Masknames[i] == maskName)
			return m_Bitmaps[i];

	wxImage image, maskimage;
	/* Translate directory seperator from ODF(\) to native format */
	wxString temp = filename;
	temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
	temp = m_organfile->GetODFPath() + wxFileName::GetPathSeparator() + temp;
	
	if (!image.LoadFile(temp, wxBITMAP_TYPE_ANY, -1))
		throw wxString::Format(_("Failed to open the graphic '%s'"), filename.c_str());

	if (maskName != wxEmptyString)
	{
		temp = maskName;
		temp.Replace(wxT("\\"), wxString(wxFileName::GetPathSeparator()));
		temp = m_organfile->GetODFPath() + wxFileName::GetPathSeparator() + temp;

		if (!maskimage.LoadFile(temp, wxBITMAP_TYPE_ANY, -1))
			throw wxString::Format(_("Failed to open the graphic '%s'"), maskName.c_str());

		if (image.GetWidth() != maskimage.GetWidth() ||
		    image.GetHeight() != maskimage.GetHeight())
			throw wxString::Format(_("bitmap size of '%s' does not match mask '%s'"), filename.c_str(), maskName.c_str());

		image.SetMaskFromImage(maskimage, 0xFF, 0xFF, 0xFF);
	}
	
	wxBitmap *bitmap = new wxBitmap(image, -1);
	RegisterBitmap(bitmap, filename, maskName);
	return bitmap;
}
