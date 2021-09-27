/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUETEMPERAMENTLIST_H
#define GORGUETEMPERAMENTLIST_H

#include "ptrvector.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueTemperament;
class GOrgueTemperamentUser;

class GOrgueTemperamentList
{
private:
	ptr_vector<GOrgueTemperament> m_Temperaments;
	ptr_vector<GOrgueTemperamentUser> m_UserTemperaments;

public:
	GOrgueTemperamentList();
	~GOrgueTemperamentList();

	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);
	ptr_vector<GOrgueTemperamentUser>& GetUserTemperaments();

	void InitTemperaments();

	const GOrgueTemperament& GetTemperament(wxString Name);
	const GOrgueTemperament& GetTemperament(unsigned index);
	unsigned GetTemperamentIndex(wxString name);
	const wxString GetTemperamentName(unsigned index);
	unsigned GetTemperamentCount();

};

#endif

