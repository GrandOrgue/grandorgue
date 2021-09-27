/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGFILE_H
#define GORGUESETTINGFILE_H

#include "GOrgueSettingString.h"

class GOrgueSettingFile : public GOrgueSettingString
{
protected:
	wxString validate(wxString value);

public:
	GOrgueSettingFile(GOrgueSettingStore* store, wxString group, wxString name, wxString default_value);
};

#endif
