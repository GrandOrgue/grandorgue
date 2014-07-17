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

#include "GOrgueBitmapCache.h"

#include "GOrguePath.h"
#include "GrandOrgueFile.h"
#include "Images.h"
#include <wx/intl.h>
#include <wx/image.h>

#define BITMAP_LIST \
	WOOD(01, 02) \
	WOOD(03, 04) \
	WOOD(05, 06) \
	WOOD(07, 08) \
	WOOD(09, 10) \
	WOOD(11, 12) \
	WOOD(13, 14) \
	WOOD(15, 16) \
	WOOD(17, 18) \
	WOOD(19, 20) \
	WOOD(21, 22) \
	WOOD(23, 24) \
	WOOD(25, 26) \
	WOOD(27, 28) \
	WOOD(29, 30) \
	WOOD(31, 32) \
	WOOD(33, 34) \
	WOOD(35, 36) \
	WOOD(37, 38) \
	WOOD(39, 40) \
	WOOD(41, 42) \
	WOOD(43, 44) \
	WOOD(45, 46) \
	WOOD(47, 48) \
	WOOD(49, 50) \
	WOOD(51, 52) \
	WOOD(53, 54) \
	WOOD(55, 56) \
	WOOD(57, 58) \
	WOOD(59, 60) \
	WOOD(61, 62) \
	WOOD(63, 64) \
	STOP(01) \
	STOP(02) \
	STOP(03) \
	STOP(04) \
	STOP(05) \
	STOP(06) \
	PISTON(01) \
	PISTON(02) \
	PISTON(03) \
	PISTON(04) \
	PISTON(05) \
	LABEL(01) \
	LABEL(02) \
	LABEL(03) \
	LABEL(04) \
	LABEL(05) \
	LABEL(06) \
	LABEL(07) \
	LABEL(08) \
	LABEL(10) \
	LABEL(11) \
	LABEL(12) \
	ENCLOSURE(A) \
	ENCLOSURE(B) \
	ENCLOSURE(C) \
	ENCLOSURE(D) \
	PEDAL(, Wood, Black) \
	PEDAL(Inverted, Black, Wood) \
	MANUAL(, White, Black) \
	MANUAL(Inverted, Black, White) \
	MANUAL(Wood, Wood, Black) \
	MANUAL(InvertedWood, Black, Wood) \

#define ENCLOSURE(A) ENC(A, 00) ENC(A, 01) ENC(A, 02) ENC(A, 03) ENC(A, 04) ENC(A, 05) ENC(A, 06) ENC(A, 07) \
	ENC(A, 08) ENC(A, 09) ENC(A, 10) ENC(A, 11) ENC(A, 12) ENC(A, 13) ENC(A, 14) ENC(A, 15)
#define WOOD(A, B) \
	DECLARE_IMAGE(Wood ## A, "GO:wood" #A); \
	DECLARE_IMAGE_ROT(Wood ## A, "GO:wood" #B);
#define STOP(A) \
	DECLARE_IMAGE(drawstop ## A ## off, "GO:drawstop" #A "_off"); \
	DECLARE_IMAGE(drawstop ## A ## on, "GO:drawstop" #A "_on");
#define PISTON(A) \
	DECLARE_IMAGE(piston ## A ## off, "GO:piston" #A "_off"); \
	DECLARE_IMAGE(piston ## A ## on, "GO:piston" #A "_on");
#define LABEL(A) \
	DECLARE_IMAGE(label ## A, "GO:label" #A);
#define ENC(A, B) \
	DECLARE_IMAGE(Enclosure ## A ## B, "GO:enclosure" #A #B);
#define PEDAL(A, B, C) \
	DECLARE_IMAGE(PedalNatural ## B ## Up, "GO:Pedal" #A "Off_Natural"); \
	DECLARE_IMAGE(PedalNatural ## B ## Down, "GO:Pedal" #A "On_Natural"); \
	DECLARE_IMAGE(PedalSharp ## C ## Up, "GO:Pedal" #A "Off_Sharp"); \
	DECLARE_IMAGE(PedalSharp ## C ## Down, "GO:Pedal" #A "On_Sharp");
#define MANUAL(A, B, C) \
	DECLARE_IMAGE(ManualC ## B ## Up, "GO:Manual" #A "Off_C"); \
	DECLARE_IMAGE(ManualC ## B ## Down, "GO:Manual" #A "On_C"); \
	DECLARE_IMAGE(ManualD ## B ## Up, "GO:Manual" #A "Off_D"); \
	DECLARE_IMAGE(ManualD ## B ## Down, "GO:Manual" #A "On_D"); \
	DECLARE_IMAGE(ManualE ## B ## Up, "GO:Manual" #A "Off_E"); \
	DECLARE_IMAGE(ManualE ## B ## Down, "GO:Manual" #A "On_E"); \
	DECLARE_IMAGE(ManualNatural ## B ## Up, "GO:Manual" #A "Off_Natural"); \
	DECLARE_IMAGE(ManualNatural ## B ## Down, "GO:Manual" #A "On_Natural"); \
	DECLARE_IMAGE(ManualSharp ## C ## Up, "GO:Manual" #A "Off_Sharp"); \
	DECLARE_IMAGE(ManualSharp ## C ## Down, "GO:Manual" #A "On_Sharp");

#define DECLARE_IMAGE(A, B) wxImage GetImage_ ## A ();
#define DECLARE_IMAGE_ROT(A, B)

BITMAP_LIST

#undef DECLARE_IMAGE
#undef DECLARE_IMAGE_ROT

#define DECLARE_IMAGE(A, B) RegisterBitmap(new wxImage(GetImage_ ## A()), wxT(B));
#define DECLARE_IMAGE_ROT(A, B) \
	static wxImage A ## _r(GetImage_ ## A().Rotate90()); \
	RegisterBitmap(new wxImage(A ## _r), wxT(B)); 

GOrgueBitmapCache::GOrgueBitmapCache(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Bitmaps(),
	m_Filenames(),
	m_Masknames()
{
	BITMAP_LIST;
}

GOrgueBitmapCache::~GOrgueBitmapCache()
{
}

void GOrgueBitmapCache::RegisterBitmap(wxImage* bitmap, wxString filename, wxString maskname)
{
	m_Bitmaps.push_back(bitmap);
	m_Filenames.push_back(filename);
	m_Masknames.push_back(maskname);
}

GOrgueBitmap GOrgueBitmapCache::GetBitmap(wxString filename, wxString maskName)
{
	for(unsigned i = 0; i < m_Filenames.size(); i++)
		if (m_Filenames[i] == filename && m_Masknames[i] == maskName)
			return GOrgueBitmap(m_Bitmaps[i]);

	wxImage image, maskimage;
	
	if (!image.LoadFile(GOCreateFilename(m_organfile, filename), wxBITMAP_TYPE_ANY, -1))
		throw wxString::Format(_("Failed to open the graphic '%s'"), filename.c_str());

	if (maskName != wxEmptyString)
	{
		if (!maskimage.LoadFile(GOCreateFilename(m_organfile, maskName), wxBITMAP_TYPE_ANY, -1))
			throw wxString::Format(_("Failed to open the graphic '%s'"), maskName.c_str());

		if (image.GetWidth() != maskimage.GetWidth() ||
		    image.GetHeight() != maskimage.GetHeight())
			throw wxString::Format(_("bitmap size of '%s' does not match mask '%s'"), filename.c_str(), maskName.c_str());

		image.SetMaskFromImage(maskimage, 0xFF, 0xFF, 0xFF);
	}
	
	wxImage* bitmap = new wxImage(image);
	RegisterBitmap(bitmap, filename, maskName);
	return GOrgueBitmap(bitmap);
}
