/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEFONT_H
#define GORGUEFONT_H

#include <wx/font.h>
#include <wx/string.h>

class GOrgueFont
{
private:
	wxFont m_Font;
	wxFont m_ScaledFont;
	wxString m_Name;
	unsigned m_Points;
	double m_Scale;

public:
	GOrgueFont();

	void SetName(const wxString& name);
	void SetPoints(unsigned points);
	wxFont GetFont(double scale);
};

#endif
