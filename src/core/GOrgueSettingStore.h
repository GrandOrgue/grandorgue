/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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
