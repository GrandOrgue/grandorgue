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
 */

#include "Images.h"
#include "GOrgueBitmapCache.h"
#include "GOrguePath.h"
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

	RegisterBitmap(new wxBitmap(GetImage_PedalNaturalWoodUp()), wxT("GO:PedalOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_PedalSharpBlackUp()), wxT("GO:PedalOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_PedalNaturalWoodDown()), wxT("GO:PedalOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_PedalSharpBlackDown()), wxT("GO:PedalOn_Sharp"));

	RegisterBitmap(new wxBitmap(GetImage_PedalNaturalBlackUp()), wxT("GO:PedalInvertedOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_PedalSharpWoodUp()), wxT("GO:PedalInvertedOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_PedalNaturalBlackDown()), wxT("GO:PedalInvertedOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_PedalSharpWoodDown()), wxT("GO:PedalInvertedOn_Sharp"));

	RegisterBitmap(new wxBitmap(GetImage_ManualCWhiteUp()), wxT("GO:ManualOff_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDWhiteUp()), wxT("GO:ManualOff_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEWhiteUp()), wxT("GO:ManualOff_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalWhiteUp()), wxT("GO:ManualOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpBlackUp()), wxT("GO:ManualOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_ManualCWhiteDown()), wxT("GO:ManualOn_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDWhiteDown()), wxT("GO:ManualOn_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEWhiteDown()), wxT("GO:ManualOn_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalWhiteDown()), wxT("GO:ManualOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpBlackDown()), wxT("GO:ManualOn_Sharp"));

	RegisterBitmap(new wxBitmap(GetImage_ManualCBlackUp()), wxT("GO:ManualInvertedOff_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDBlackUp()), wxT("GO:ManualInvertedOff_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEBlackUp()), wxT("GO:ManualInvertedOff_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalBlackUp()), wxT("GO:ManualInvertedOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpWhiteUp()), wxT("GO:ManualInvertedOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_ManualCBlackDown()), wxT("GO:ManualInvertedOn_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDBlackDown()), wxT("GO:ManualInvertedOn_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEBlackDown()), wxT("GO:ManualInvertedOn_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalBlackDown()), wxT("GO:ManualInvertedOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpWhiteDown()), wxT("GO:ManualInvertedOn_Sharp"));

	RegisterBitmap(new wxBitmap(GetImage_ManualCWoodUp()), wxT("GO:ManualWoodOff_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDWoodUp()), wxT("GO:ManualWoodOff_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEWoodUp()), wxT("GO:ManualWoodOff_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalWoodUp()), wxT("GO:ManualWoodOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpBlackUp()), wxT("GO:ManualWoodOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_ManualCWoodDown()), wxT("GO:ManualWoodOn_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDWoodDown()), wxT("GO:ManualWoodOn_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEWoodDown()), wxT("GO:ManualWoodOn_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalWoodDown()), wxT("GO:ManualWoodOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpBlackDown()), wxT("GO:ManualWoodOn_Sharp"));

	RegisterBitmap(new wxBitmap(GetImage_ManualCBlackUp()), wxT("GO:ManualInvertedWoodOff_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDBlackUp()), wxT("GO:ManualInvertedWoodOff_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEBlackUp()), wxT("GO:ManualInvertedWoodOff_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalBlackUp()), wxT("GO:ManualInvertedWoodOff_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpWoodUp()), wxT("GO:ManualInvertedWoodOff_Sharp"));
	RegisterBitmap(new wxBitmap(GetImage_ManualCBlackDown()), wxT("GO:ManualInvertedWoodOn_C"));
	RegisterBitmap(new wxBitmap(GetImage_ManualDBlackDown()), wxT("GO:ManualInvertedWoodOn_D"));
	RegisterBitmap(new wxBitmap(GetImage_ManualEBlackDown()), wxT("GO:ManualInvertedWoodOn_E"));
	RegisterBitmap(new wxBitmap(GetImage_ManualNaturalBlackDown()), wxT("GO:ManualInvertedWoodOn_Natural"));
	RegisterBitmap(new wxBitmap(GetImage_ManualSharpWoodDown()), wxT("GO:ManualInvertedWoodOn_Sharp"));
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
	
	if (!image.LoadFile(GOCreateFilename(m_organfile->GetODFPath(), filename), wxBITMAP_TYPE_ANY, -1))
		throw wxString::Format(_("Failed to open the graphic '%s'"), filename.c_str());

	if (maskName != wxEmptyString)
	{
		if (!maskimage.LoadFile(GOCreateFilename(m_organfile->GetODFPath(), maskName), wxBITMAP_TYPE_ANY, -1))
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
