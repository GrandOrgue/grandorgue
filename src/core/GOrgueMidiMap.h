/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDIMAP_H
#define GORGUEMIDIMAP_H

#include <wx/string.h>
#include <vector>

class GOrgueMidiMap
{
private:
	std::vector<wxString> m_DeviceMap;
	std::vector<wxString> m_ElementMap;

public:
	GOrgueMidiMap();
	~GOrgueMidiMap();

	unsigned GetDeviceByString(const wxString& str);
	const wxString& GetDeviceByID(unsigned id);

	unsigned GetElementByString(const wxString& str);
	const wxString& GetElementByID(unsigned id);
};

#endif
