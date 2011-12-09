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

#ifndef GORGUEFRAMEGENERAL_H
#define GORGUEFRAMEGENERAL_H

#include <wx/wx.h>
#include <vector>

class GrandOrgueFile;
class IniFileConfig;

class GOrgueFrameGeneral
{
private:
	GrandOrgueFile* m_organfile;
	wxString m_group;
	std::vector<int> m_Stops;
	std::vector<unsigned> m_StopManual;
	std::vector<int> m_Couplers;
	std::vector<unsigned> m_CouplerManual;
	std::vector<int> m_Tremulants;
	std::vector<int> m_DivisionalCouplers;
	bool m_Protected;

public:
	GOrgueFrameGeneral(GrandOrgueFile* organfile);
	void Load(IniFileConfig& cfg, wxString group);
	void LoadCombination(IniFileConfig& cfg);
	void Save(IniFileConfig& cfg, bool prefix);
	void Copy(GOrgueFrameGeneral* general);
	void Push();

};

#endif /* GORGUEFRAMEGENERAL_H */
