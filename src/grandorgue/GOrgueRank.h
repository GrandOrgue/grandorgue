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

#ifndef GORGUERANK_H
#define GORGUERANK_H

#include "ptrvector.h"
#include "GOrguePipeConfig.h"

class GOrguePipe;
class GOrgueStop;
class GOrgueTemperament;
class GrandOrgueFile;

class GOrgueRank : public GOrguePipeUpdateCallback
{
private:
	GrandOrgueFile* m_organfile;
	wxString m_Name;
	wxString m_Group;
	ptr_vector<GOrguePipe> m_Pipes;
	unsigned m_StopCount;
	std::vector<unsigned> m_Velocity;
	std::vector<std::vector<unsigned> > m_Velocities;
	unsigned m_FirstMidiNoteNumber;
	bool m_Percussive;
	unsigned m_WindchestGroup;
	unsigned m_HarmonicNumber;
	float m_PitchCorrection;
	float m_MinVolume;
	float m_MaxVolume;
	GOrguePipeConfig m_PipeConfig;

	void Resize();

public:
	GOrgueRank(GrandOrgueFile* organfile);
	~GOrgueRank();
	void Load(GOrgueConfigReader& cfg, wxString group, int first_midi_note_number);
	void Save(GOrgueConfigWriter& cfg);
	unsigned RegisterStop(GOrgueStop* stop);
	void SetKey(int note, unsigned velocity, unsigned stopID);
	GOrguePipe* GetPipe(unsigned index);
	unsigned GetPipeCount();
	void Abort();
	void PreparePlayback();
	GOrguePipeConfig& GetPipeConfig();
	void SetTemperament(const GOrgueTemperament& temperament);
	const wxString& GetName();

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateAudioGroup();
};

#endif
