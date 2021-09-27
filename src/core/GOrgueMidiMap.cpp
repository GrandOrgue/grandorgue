/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueMidiMap.h"

#include <assert.h>

GOrgueMidiMap::GOrgueMidiMap() :
	m_DeviceMap(),
	m_ElementMap()
{
	m_DeviceMap.push_back(wxEmptyString);
	m_ElementMap.push_back(wxEmptyString);
	assert(m_DeviceMap[0] == wxEmptyString);
}

GOrgueMidiMap::~GOrgueMidiMap()
{
}

const wxString& GOrgueMidiMap::GetDeviceByID(unsigned id)
{
	assert(id <= m_DeviceMap.size());
	return m_DeviceMap[id];
}

unsigned GOrgueMidiMap::GetDeviceByString(const wxString& str)
{
	for(unsigned i = 0; i < m_DeviceMap.size(); i++)
		if (m_DeviceMap[i] == str)
			return i;
	m_DeviceMap.push_back(str);
	return m_DeviceMap.size() - 1;
}

unsigned  GOrgueMidiMap::GetElementByString(const wxString& str)
{
	for(unsigned i = 0; i < m_ElementMap.size(); i++)
		if (m_ElementMap[i] == str)
			return i;
	m_ElementMap.push_back(str);
	return m_ElementMap.size() - 1;
}

const wxString&  GOrgueMidiMap::GetElementByID(unsigned id)
{
	assert(id <= m_ElementMap.size());
	return m_ElementMap[id];
}
