/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2017 GrandOrgue contributors (see AUTHORS)
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

#ifndef SETTINGSDEFAULTS_H
#define SETTINGSDEFAULTS_H

#include <wx/panel.h>

class GOrgueSettings;
class wxDirPickerCtrl;
class wxSpinCtrl;

class SettingsDefaults : public wxPanel
{
	enum {
		ID_ORGAN_PATH = 200,
		ID_ORGANPACKAGE_PATH,
		ID_SETTING_PATH,
		ID_AUDIO_RECORDER_PATH,
		ID_MIDI_RECORDER_PATH,
		ID_MIDI_PLAYER_PATH,
		ID_METRONOME_MEASURE,
		ID_METRONOME_BPM,
		ID_VOLUME,
	};
private:
	GOrgueSettings& m_Settings;
	wxDirPickerCtrl* m_OrganPath;
	wxDirPickerCtrl* m_OrganPackagePath;
	wxDirPickerCtrl* m_SettingPath;
	wxDirPickerCtrl* m_AudioRecorderPath;
	wxDirPickerCtrl* m_MidiRecorderPath;
	wxDirPickerCtrl* m_MidiPlayerPath;
	wxSpinCtrl* m_MetronomeMeasure;
	wxSpinCtrl* m_MetronomeBPM;
	wxSpinCtrl* m_Volume;

public:
	SettingsDefaults(GOrgueSettings& settings, wxWindow* parent);

	void Save();
};

#endif
