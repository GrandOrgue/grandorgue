/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTINGSTORE_H
#define GORGUESETTINGSTORE_H

#include <vector>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueSetting;

class GOrgueSettingStore
{
private:
	std::vector<GOrgueSetting*> m_SettingList;

protected:
	void Load(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

public:
	GOrgueSettingStore();
	virtual ~GOrgueSettingStore();

	void AddSetting(GOrgueSetting* setting);
};

#endif
