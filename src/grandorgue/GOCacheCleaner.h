/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOCACHECLEANER_H
#define GOCACHECLEANER_H

class GOSettings;
class wxArrayString;

class GOCacheCleaner
{
private:
	GOSettings& m_settings;

	wxArrayString GetOrganIDList();
	wxArrayString GetArchiveIDList();
	
public:
	GOCacheCleaner(GOSettings& settings);
	~GOCacheCleaner();

	void Cleanup();
};

#endif
