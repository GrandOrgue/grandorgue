/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "GOrgueWindchest.h"
#include "GOrgueEnclosure.h"
#include "GrandOrgueFile.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;

GOrgueWindchest::GOrgueWindchest() :
	NumberOfEnclosures(0),
	NumberOfTremulants(0),
	enclosure(),
	tremulant()
{

	for(unsigned int i = 0; i < MAX_TREMULANTS; i++)
		tremulant[i]=0;

	for(unsigned int i = 0; i < MAX_ENCLOSURES; i++)
		enclosure[i]=0;

}


void GOrgueWindchest::Load(IniFileConfig& cfg, wxString group)
{

	int i;
	wxString buffer;

	NumberOfEnclosures = cfg.ReadInteger(group, wxT("NumberOfEnclosures"), 0, 6);
	NumberOfTremulants = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, 6);

	for (i = 0; i < NumberOfEnclosures; i++)
	{
		buffer.Printf(wxT("Enclosure%03d"), i + 1);
		enclosure[i] = cfg.ReadInteger(group, buffer, 1, organfile->GetEnclosureCount()) - 1;
	}
	for (i = 0; i < NumberOfTremulants; i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		tremulant[i] = cfg.ReadInteger(group, buffer, 1, organfile->GetTremulantCount()) - 1;
	}

}

double GOrgueWindchest::GetVolume()
{
	double d = 1.0;
	for (int i = 0; i < NumberOfEnclosures; i++)
		d  *= organfile->GetEnclosure(enclosure[i])->GetAttenuation();
	return d;
}

int GOrgueWindchest::GetTremulantCount()
{
	return NumberOfTremulants;
}
