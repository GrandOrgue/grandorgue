/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#ifndef SETTINGSOPTION_H
#define SETTINGSOPTION_H

#include <wx/wx.h>
class wxDirPickerCtrl;

class GOrgueSettings;

class SettingsOption : public wxPanel
{
	enum {
		ID_WAVE_FORMAT = 200,
		ID_CONCURRENCY,
		ID_RELEASE_CONCURRENCY,
		ID_ENHANCE_SQUASH,
		ID_ENHANCE_MANAGE_POLYPHONY,
		ID_COMPRESS_CACHE,
		ID_ENHANCE_SCALE_RELEASE,
		ID_ENHANCE_RANDOMIZE,
		ID_SETTINGS_DIR,
		ID_CACHE_DIR,
	};
private:
	GOrgueSettings& m_Settings;
	wxChoice* m_Concurrency;
	wxChoice* m_ReleaseConcurrency;
	wxChoice* m_WaveFormat;
	wxCheckBox* m_Squash;
	wxCheckBox* m_Limit;
	wxCheckBox* m_CompressCache;
	wxCheckBox* m_Scale;
	wxCheckBox* m_Random;
	wxDirPickerCtrl* m_SettingsPath;
	wxDirPickerCtrl* m_CachePath;
	bool m_OldSquash;

public:
	SettingsOption(GOrgueSettings& settings, wxWindow* parent);

	bool NeedReload();

	void Save();
};

#endif
