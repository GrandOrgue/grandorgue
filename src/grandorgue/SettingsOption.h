/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
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

#ifndef SETTINGSOPTION_H
#define SETTINGSOPTION_H

#include <wx/panel.h>

class GOrgueSettings;
class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxSpinCtrl;

class SettingsOption : public wxPanel
{
	enum {
		ID_WAVE_FORMAT = 200,
		ID_CONCURRENCY,
		ID_RELEASE_CONCURRENCY,
		ID_LOAD_CONCURRENCY,
		ID_LOSSLESS_COMPRESSION,
		ID_MANAGE_POLYPHONY,
		ID_COMPRESS_CACHE,
		ID_MANAGE_CACHE,
		ID_SCALE_RELEASE,
		ID_LOAD_LAST_FILE,
		ID_RANDOMIZE,
		ID_SETTINGS_DIR,
		ID_CACHE_DIR,
		ID_BITS_PER_SAMPLE,
		ID_LOOP_LOAD,
		ID_ATTACK_LOAD,
		ID_RELEASE_LOAD,
		ID_MONO_STEREO,
		ID_INTERPOLATION,
		ID_SAMPLE_RATE,
		ID_SAMPLES_PER_BUFFER,
		ID_MEMORY_LIMIT,
		ID_ODF_CHECK,
	};
private:
	GOrgueSettings& m_Settings;
	wxChoice* m_Concurrency;
	wxChoice* m_ReleaseConcurrency;
	wxChoice* m_LoadConcurrency;
	wxChoice* m_WaveFormat;
	wxCheckBox* m_LosslessCompression;
	wxCheckBox* m_Limit;
	wxCheckBox* m_CompressCache;
	wxCheckBox* m_ManageCache;
	wxCheckBox* m_LoadLastFile;
	wxCheckBox* m_Scale;
	wxCheckBox* m_Random;
	wxCheckBox* m_ODFCheck;
	wxDirPickerCtrl* m_SettingsPath;
	wxDirPickerCtrl* m_CachePath;
	wxChoice* m_BitsPerSample;
	wxChoice* m_LoopLoad;
	wxChoice* m_AttackLoad;
	wxChoice* m_ReleaseLoad;
	wxChoice* m_Stereo;
	wxChoice* m_Interpolation;
	wxChoice* m_SampleRate;
	wxSpinCtrl* m_SamplesPerBuffer;
	wxSpinCtrl* m_MemoryLimit;
	bool m_OldStereo;
	bool m_OldLosslessCompression;
	unsigned m_OldBitsPerSample;
	unsigned m_OldLoopLoad;
	unsigned m_OldAttackLoad;
	unsigned m_OldReleaseLoad;

public:
	SettingsOption(GOrgueSettings& settings, wxWindow* parent);

	bool NeedReload();

	void Save();
};

#endif
