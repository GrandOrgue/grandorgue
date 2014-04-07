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

#include "GOrgueTemperamentList.h"

#include "GOrgueConfigReader.h"
#include "GOrgueConfigWriter.h"
#include "GOrgueLimits.h"
#include "GOrgueTemperament.h"
#include "GOrgueTemperamentCent.h"
#include "GOrgueTemperamentUser.h"
#include <wx/intl.h>

GOrgueTemperamentList::GOrgueTemperamentList()
{
}

GOrgueTemperamentList::~GOrgueTemperamentList()
{
}

void GOrgueTemperamentList::Load(GOrgueConfigReader& cfg)
{
	unsigned count = cfg.ReadInteger(CMBSetting, wxT("General"), wxT("UserTemperamentCount"), 0, MAX_TEMPERAMENTS, false, 0);
	for(unsigned i = 0; i < count; i++)
		m_UserTemperaments.push_back(new GOrgueTemperamentUser(cfg, wxString::Format(wxT("UserTemperament%03d"), i + 1)));;
}

void GOrgueTemperamentList::Save(GOrgueConfigWriter& cfg)
{
	if (m_UserTemperaments.size() > MAX_TEMPERAMENTS)
		m_UserTemperaments.resize(MAX_TEMPERAMENTS);

	for(unsigned i = 0; i < m_UserTemperaments.size(); i++)
		m_UserTemperaments[i]->Save(cfg, wxString::Format(wxT("UserTemperament%03d"), i + 1));

	cfg.WriteInteger(wxT("General"), wxT("UserTemperamentCount"), m_UserTemperaments.size());
}

ptr_vector<GOrgueTemperamentUser>& GOrgueTemperamentList::GetUserTemperaments()
{
	return m_UserTemperaments;
}

const GOrgueTemperament& GOrgueTemperamentList::GetTemperament(unsigned index)
{
	if (index < m_Temperaments.size())
		return *m_Temperaments[index];

	index -= m_Temperaments.size();
	if (index < m_UserTemperaments.size())
		return *m_UserTemperaments[index];

	/* else return original temperament */
	return *m_Temperaments[0];
}


const GOrgueTemperament& GOrgueTemperamentList::GetTemperament(wxString Name)
{
	for(unsigned i = 0; i < m_Temperaments.size(); i++)
		if (Name == m_Temperaments[i]->GetName())
			return *m_Temperaments[i];

	for(unsigned i = 0; i < m_UserTemperaments.size(); i++)
		if (Name == m_UserTemperaments[i]->GetName())
			return *m_UserTemperaments[i];

	/* else return original temperament */
	return *m_Temperaments[0];
}

unsigned GOrgueTemperamentList::GetTemperamentIndex(wxString name)
{
	for(unsigned i = 0; i < m_Temperaments.size(); i++)
		if (m_Temperaments[i]->GetName() == name)
			return i;

	for(unsigned i = 0; i < m_UserTemperaments.size(); i++)
		if (m_UserTemperaments[i]->GetName() == name)
			return i + m_Temperaments.size();

	return 0;
}

const wxString GOrgueTemperamentList::GetTemperamentName(unsigned index)
{
	if (index < m_Temperaments.size())
		return m_Temperaments[index]->GetName();

	index -= m_Temperaments.size();
	if (index < m_UserTemperaments.size())
		return m_UserTemperaments[index]->GetName();

	return m_Temperaments[0]->GetName();
}

unsigned GOrgueTemperamentList::GetTemperamentCount()
{
	return m_Temperaments.size() + m_UserTemperaments.size();
}

void GOrgueTemperamentList::InitTemperaments()
{
	wxString group;

	if (m_Temperaments.size())
		return;

	group = wxT("");
	m_Temperaments.push_back(new GOrgueTemperament(wxTRANSLATE("Original temperament")));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Equal temperament"), group, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/4 comma meantone (Aaron 1523)"), group, 10.26, -13.69, 3.42, 20.52, -3.43, 13.68, -10.27, 6.84, -17.11, 0, 17.1, -6.85));

	group = _("1/4 comma variants");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/4-comma meantone approximation"), group, 15.64, -7.4, 8.2, 31.28, 1.95, 13.69, -1.85, 17.6, -11.73, 0, 23.08, -1.12));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/4-comma equal beating fifths"), group, 7.91, -6.37, 2.85, 13.58, -1.25, 9.98, -4.5, 4.49, -9.53, 0, 11.03, -3.59));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Alexander Metcalf Fisher"), group, 10.26, -13.69, 3.42, 7.75, -3.42, 12.69, -10.26, 6.84, -9.67, 0, 15.12, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Chaumont"), group, 10.265, -13.686, 3.422, 1.174, -3.421, 13.687, -10.264, 6.843, -17.108, 0, 7.43, -6.843));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Corrette (1753)"), group, 11.73001, -15.64, 3.91001, 7.82001, -3.909996923, 15.64001, -11.73, 7.82001, -11.73, 0, 11.73001, -7.82));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Couperin"), group, 10.26471, -13.68629, 3.42157, 7.36758, -3.42129, 13.68628, -10.26472, 6.84314, -6.35472, 0, 17.10785, -6.84315));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("d'Alembert modified meantone (1726)"), group, 10.26, -11.59, 3.42, -8.16, -3.42, 4.12, -8.86, 6.84, -14.31, 0, -2.02, -6.14));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Ferdinand Bossard (1743/44) Klosterkirche Muri"), group, 8.8, -10.75, 3.91, 14.66, -2.93, 10.75, -8.8, 7.82, -11.73, 0, 9.77, -5.86));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Fisk-Vogel: Memorial Church at Stanford"), group, 8.21, -12.32, 2.74, 17.79, -2.74, 10.95, -8.9, 5.47, -15.74, 0, 14.37, -5.48));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mersenne's Improved Meantone 1"), group, 10.26, -13.69, 3.42, 9.78, -3.42, 13.69, -10.26, 6.84, -17.11, 0, 11.73, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Rameau bemols"), group, 10.26, 2.93, 3.42, 15.15, -3.42, 13.69, -7.53, 6.84, 10.26, 0, 17.11, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Rameau dieses"), group, 10.26, -13.69, 3.42, -4.13, -3.42, 8.31, -10.26, 6.84, -14.42, 0, 3.47, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Rameau's modified meantone temperament (1725)"), group, 10.27, -2.93, 3.42, 8.07, -3.42, 13.69, -4.89, 6.84, -0.98, 0, 17.11, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Rameau Nouveau Systeme (1726)"), group, 10.26, 2.74, 3.42, 12.32, -3.42, 13.69, -2.05, 6.84, 7.53, 0, 17.11, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Stade St. Cosmae"), group, 10.26471, -8.30972, 3.42157, 0, -3.42129, 8.30971, -10.26472, 6.84314, -6.35472, 0, 6.35471, -6.84315));

	group = _("1/5 comma");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/5-comma meantone (Verheijen)"), group, 7.04, -9.39, 2.35, 14.08, -2.35, 9.38, -7.04, 4.69, -11.73, 0, 11.73, -4.69));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Holden (John) 1770"), group, 7.04, 18.77, 2.35, 14.08, -2.35, 9.38, -7.04, 4.69, 16.42, 0, 11.73, 14.86));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Scheffer (1748) modified, Sweden"), group, 7.04, -9.39, 2.35, 0, -2.35, 9.38, -7.04, 4.69, -11.73, 0, 11.73, -4.69));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Keller (Gottfried) 1707"), group, 8.16, -10.89, 2.72, 16.33, -2.72, 10.89, -8.16, 5.44, -13.61, 0, 13.61, -5.44));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Schnitger organ St. Jakobi, Hamburg "), group, 7.04, -5.08, 2.35, 3.13, -2.35, 9.39, -7.04, 4.69, -3.13, 0, 7.43, -4.69));

	group = _("1/6 comma");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/6-comma meantone"), group, 5.87, -3.91, 1.96, 0, -1.95, 7.82, -3.91, 3.91, -1.96, 0, 5.87, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/6-comma (bemols)"), group, 4.89, -6.52, 1.63, 9.78, -1.63, 6.52, -4.89, 3.26, 11.39, 0, 8.15, -3.26));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/6-comma (dieses)"), group, 4.89, -6.52, 1.63, 9.78, -1.63, 6.52, -4.89, 3.26, -8.15, 0, 8.15, -3.26));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/6-comma equal beating fifths"), group, 3.31, -1.61, 1.24, 5.15, -0.18, 4.04, -1.02, 1.68, -3.04, 0, 4.09, -1.07));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("1/6-comma, wolf over 4 fifths"), group, 4.89, -1.63, 1.63, 0, -1.63, 6.52, -4.89, 3.26, 1.63, 0, 3.26, -3.26));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Freytag Organ Bellingwolde 1798/2002"), group, 5.87, -3.91, 1.96, 7.82, -1.95, 7.82, -5.87, 3.91, 1.96, 0, 5.87, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("G. Silbermann nr 2"), group, 5.87, -7.82, 1.96, 11.73, -1.95, 7.82, -5.87, 3.91, -9.77, 0, 9.77, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sauveur (Memoires 1701)"), group, 5.46, 14.55, 1.82, 10.91, -1.82, 7.27, 16.36, 3.64, 12.73, 0, 9.09, -3.64));

	group = _("Various comma");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("3/14-comma meantone (Giordano Riccati, 1762)"), group, 7.96, -10.61, 2.65, 15.92, -2.65, 10.61, -7.96, 5.31, -13.27, 0, 13.27, -5.31));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Lambert (1774) 1/7 comma"), group, 4.19, -2.24, 1.4, 1.67, -1.4, 5.58, -4.19, 2.79, -0.28, 0, 3.63, -2.79));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Variable meantone 1"), group, 10.26, -8.31, 3.42, 15.15, -3.42, 11.89, -6.68, 6.84, -9.94, 0, 13.52, -5.05));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Variable meantone 2"), group, 10.26, -8.51, 3.42, 15.36, -3.42, 12.61, -7.4, 6.84, -9.25, 0, 14.24, -5.77));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Variable meantone 3"), group, 10.26, -1.14, 3.42, 7.98, -3.42, 11.89, -3.1, 6.84, 0.81, 0, 9.94, -5.05));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Variable meantone 4"), group, 10.26, -2.93, 3.42, 9.78, -3.42, 13.69, -4.89, 6.84, -0.98, 0, 11.73, -6.84));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("William Holder equal beating (1694)"), group, 9.99, -8.54, 3.58, 17.39, -1.74, 12.66, -6.08, 5.76, -12.48, 0, 14.17, -4.73));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Zarlino (1558) 2/7-comma meantone"), group, 12.57, -16.76, 4.19, 25.14, -4.19, 16.76, -12.57, 8.38, -20.95, 0, 20.95, -8.38));

	group = _("Pythagorean");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Pythagorean"), group, -5.87, 7.82, -1.96, -11.74, 1.95, -7.83, 5.86, -3.92, 9.77, 0, -9.78, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Boulliau (1373)/Mersenne 1636. "), group, -5.87, -6.91, -1.96, -3, 1.95, -7.82, -8.87, -3.91, -4.96, 0, -9.77, -10.82));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("De Caus (1615)"), group, 15.64, -13.69, -1.96, -9.78, 1.95, 13.69, -15.64, 17.6, -11.73, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Grammateus (1518)"), group, -5.87, -3.91, -1.96, 0, 1.95, -7.82, -5.87, -3.91, -1.96, 0, 1.96, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Henri Arnaut De Zwolle. (1436)."), group, -5.87, -15.64, -1.96, -11.73, 1.95, -7.82, -17.6, -3.91, -13.69, 0, -9.77, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sauveur 1702"), group, 15.64, -13.69, 19.55, 31.28, 1.95, 13.69, 5.87, 17.6, 29.33, 0, 33.24, 3.91));

	group = _("Other just");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Pure Major"), group, 16, 28, 20, 32, 0, 14, 6, 18, 30, 0, 12, 4));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Pure Minor"), group, 16, -13, 0, 32, 0, 14, 6, 18, 30, 0, 12, 4));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Smith's (Robert) Equal Harmony 1749"), group, 12.07, -16.09, 4.02, 24.13, -4.02, 16.09, -12.07, 8.04, -20.11, 0, 20.11, -8.04));
	
	group = _("Well tempered");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Bach (Bradley Lehman)"), group, +5.9, +3.9, +2, +3.9, -1.9, +7.9, +2, +3.9, +3.9, 0, +3.9, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Barca 1786"), group, 4.56, -3.26, 1.95, 0.65, -1.96, 2.61, -5.21, 2.61, -1.3, 0, 0.65, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Bethisy temperament ordinaire (1764)"), group, 10.26, -2.93, 3.42, -0.98, -3.42, 6.52, -2.93, 6.84, -2.93, 0, 2.77, -3.42));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Freres Jullien Organ, France (1690)"), group, 7, -11, 3, 1, -1, 4, -8, 5, -13, 0, 3, -2));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Ganassi (Sylvestro) - 1543"), group, 15.64, 4.44, -1.96, -3, 1.95, 13.69, 12.64, 17.6, 6.4, 0, -1.05, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Gottfried Silbermann nr 1"), group, 4.89, -7.82, 4.89, 12.71, -4.89, 7.82, -4.89, 4.89, -10.75, 0, 10.75, -4.89));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Heun (Jan) 1805"), group, 5.45, -7.27, 1.82, 10.91, -1.82, 7.27, -5.45, 3.64, -9.09, 0, 9.09, -3.64));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Hinsz (1704 - 1785), Pelstergasthuiskerk Groningen"), group, 11.73, -3.91, 3.91, 5.87, -3.91, 9.78, -5.87, 7.82, -1.95, 0, 7.82, -1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Kellner's Bach tuning"), group, 8.21, -1.56, 2.74, 2.35, -2.74, 6.26, -3.52, 5.47, 0.39, 0, 4.3, -0.78));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Kepler's choice system (1619)"), group, -5.87, -13.69, -1.96, 9.78, -19.55, -7.82, -15.64, -3.91, -11.73, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Kirnberger III"), group, 10.26, 0.44, 3.42, 4.39, -3.43, 8.3, 0.48, 6.84, 2.44, 0, 6.35, -1.47));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Koenig, Balthasar Organ Nederehe 1714"), group, 9, 18.5, 8, 15, -4.5, 13, 22.5, 4, 16.5, 0, 15.5, -2.5));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mercadier's well-temperament"), group, 5.87, 0, 1.96, 1.95, -1.95, 5.87, 0, 3.91, 0, 0, 3.91, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Malerbi's well-temperament nr 1 (1794)"), group, 8.21, -1.56, 2.74, 2.35, -2.74, 6.26, -3.52, 5.47, 0.39, 0, 4.3, -5.47));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mark Lindley, Grosvenor Chapel, London"), group, 5.87, -3.91, 1.96, 0, -1.95, 7.82, -3.91, 3.91, -1.96, 0, 3.91, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 1"), group, 15.64, 5.87, 19.55, 9.78, 1.95, 13.69, 5.87, 17.6, 7.82, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 2"), group, -5.87, 3.91, -1.96, 7.82, 1.95, -7.82, 1.96, -3.91, 5.87, 0, 9.77, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 3"), group, -5.87, -9.78, -1.96, -5.87, 1.95, -7.82, -11.73, -3.91, -7.82, 0, -9.77, -13.68));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 4"), group, -5.87, -7.82, -1.96, -11.73, 1.95, -7.82, -9.77, -3.91, -5.87, 0, -9.77, -11.73));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 5"), group, -5.87, -1.96, -1.96, 1.95, 1.95, -7.82, -3.91, -3.91, 0, 0, 3.91, -5.87));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 7"), group, 1.96, 0, -1.95, -3.91, 1.96, 0, -1.95, -3.91, 1.96, 0, -1.95, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 8"), group, 0, 1.95, -1.96, 0, 1.95, -1.95, 0, -3.91, -1.96, 0, 1.96, -1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 9"), group, -5.87, -3.91, -1.96, 0, 1.95, -1.95, 0, -3.91, -1.96, 0, 1.96, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 11"), group, -5.87, 0, -1.96, -3.91, 1.95, -7.82, 1.96, -3.91, -1.96, 0, -5.87, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Marpurg nr 12"), group, -7.82, 3.91, -1.96, -11.73, -1.96, -7.82, 1.95, -3.91, 5.86, 0, -9.78, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mod. Silbermann nr 2"), group, 5.87, -7.82, 1.96, 3.91, -1.95, 7.82, -5.87, 3.91, -9.77, 0, 9.77, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Neidhardt I (1724)"), group, 5.87, 0, 1.96, 1.95, -1.95, 3.91, -1.96, 3.91, 1.96, 0, 1.96, -1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Neidhardt II (1724)"), group, 5.87, 1.96, 1.96, 3.91, 0, 5.87, 1.96, 3.91, 1.96, 0, 5.87, 1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Neidhardt III (1724)"), group, 5.87, 1.96, 1.96, 3.91, 0, 3.91, 1.96, 3.91, 1.96, 0, 3.91, 1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Norden"), group, 8.211, -6.25601, 2.737, 2.346, -2.737, 10.948, -8.21101, 5.474, -4.30101, 0, 8.993, -5.104));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Prelleur (Peter) 1731"), group, 4.31, -0.45, 1.59, 3.15, -0.41, 5.33, -0.84, 2.29, 1.2, 0, 4.37, -1.62));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Rousseau's temperament (1768)"), group, 3.12, 0.83, 1.04, 3.24, -1.04, 3.66, -0.62, 2.08, 2.29, 0, 3.7, -2.08));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Schlick (Arnolt), 1511"), group, 5.87, -3.91, 1.96, 7.82, -1.95, 7.82, -3.91, 3.91, 1.96, 0, 7.82, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Schlick: 1555"), group, 5.87, -5.86, 1.96, 9.78, -1.95, 7.82, -4.89, 3.91, 4.89, 0, 8.8, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Schlick's (1991)"), group, 5.87, -5.86, 1.96, 9.78, -1.95, 7.82, -3.91, 3.91, 5.87, 0, 7.82, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sorge, 1744 (A)"), group, 3.91, 1.95, 0, 5.86, 0, 5.86, 1.95, 1.95, 3.91, 0, 5.87, 1.96));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sorge, 1744 (B)"), group, 5.87, 0, 1.96, 3.91, 1.95, 3.91, 0, 3.91, 1.96, 0, 3.91, 0)); 
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sorge, 1758"), group, 5.87, 1.96, 1.96, 3.91, 0, 3.91, 1.96, 3.91, 3.91, 0, 3.91, 1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Thomas/Philpott 1829/1881 organ, St. Jansklooster"), group, -7.82, 14.66, -1.95, 7.82, 4.89, -2.93, 13.69, -7.82, 14.66, 0, 1.94, 11.73));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Valotti"), group, 6.13, -0.18, 2.04, 4.26, -2.05, 8.17, -2.14, 4.09, 2.31, 0, 6.22, -4.09));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Wegscheider 20 note"), group, 10.26, 3.81, 3.42, 7.39, 0.95, 9.3, 2.86, 6.84, 4.77, 0, 8.35, 1.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Werckmeister III"), group, 11.73, 1.95, 3.91, 5.87, 1.96, 9.78, 0, 7.82, 3.91, 0, 7.82, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Werckmeister 4 (1691)"), group, 9.77, -7.82, 5.86, 3.91, 1.95, 7.82, -1.96, 3.91, -5.86, 0, 13.68, -3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Werckmeister 5 (1691)"), group, 0, -3.91, 3.91, 0, -3.91, 3.91, 0, 1.96, -7.82, 0, 1.96, -1.95));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Werckmeister 6"), group, 6.79, -2.55, -6.88, 4.85, 1.96, 4.83, 1.71, 4.33, -0.6, 0, 6.81, 3.91));

	group = _("Harpsichord/Piano/Monochord");
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Agricola's Monochord, Rudimenta musices (1539)"), group, -5.87, -13.69, -1.96, -9.78, 1.95, -7.82, -15.64, -3.91, -11.73, 0, -9.77, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Bermudo (1555) "), group, -2.16, -2.06, -1.95, -8.03, -1.75, -4.12, -4.01, -0.21, -0.1, 0, -6.07, 0.21));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("d'Alembert's: John Sankey's Scarlatti"), group, 11.3, -3.1, 4.7, 2.7, -2.39, 9.34, -4, 8, -1.2, 0, 6.2, -2.38));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Equal beating Victorian piano temperament"), group, 3.8, -0.77, 0.86, 3.14, -1.98, 1.84, -2.72, 3.11, 1.19, 0, 2.36, -0.03));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Hummel's quasi-equal temperament (1829)"), group, 0.5, 0.42, -0.2, -0.02, -0.39, 0.02, -0.14, 0.48, 0.52, 0, 0.28, 0));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mersenne spinet 1"), group, 15.64, 27.37, -1.95, 31.28, 1.96, 13.69, 25.42, 17.6, 29.33, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Mersenne spinet 2"), group, 15.64, -13.69, 19.55, -9.78, 1.95, 13.69, -15.64, 17.6, -11.73, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Ramos de Pareja 1482 - Monochord"), group, 15.64, 7.82, -1.96, 9.78, 1.95, 13.69, 5.87, 17.6, 7.82, 0, 11.73, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sauveur harpsichord 1697"), group, 6.71, -7.86, 1.03, 18.5, 2.4, 9.04, -6.07, 3.87, 14.42, 0, 16.63, -3.01));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Sorge Monochord (1756)"), group, 15.64, -13.69, 19.55, -9.78, 1.95, 13.69, 5.87, 17.6, -11.73, 0, 33.24, 3.91));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Stevin (Simon), monochord 1585"), group, -0.59, -0.46, -0.6, -0.6, -0.37, -0.47, -0.58, -0.54, -0.15, 0, -0.19, -0.16));
	m_Temperaments.push_back(new GOrgueTemperamentCent(wxTRANSLATE("Victorian temperament 1885"), group, 5, 1, 3, 3, -2, 5, 0, 5, 2, 0, 4, -1));
}
