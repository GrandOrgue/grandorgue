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

#ifndef GORGUEPIPE_H
#define GORGUEPIPE_H

#include "GOrgueCacheObject.h"
#include "GOrguePipeConfigNode.h"
#include "GOrguePipeWindchestCallback.h"
#include <vector>
#include <wx/string.h>

class GOrgueCache;
class GOrgueCacheWriter;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueRank;
class GOrgueTemperament;
class GrandOrgueFile;

class GOrguePipe : private GOrguePipeUpdateCallback, protected GOrgueCacheObject, protected GOrguePipeWindchestCallback
{
private:
	unsigned m_Velocity;
	std::vector<unsigned> m_Velocities;

protected:
	GrandOrgueFile* m_organfile;
	GOrgueRank* m_Rank;

	wxString m_Filename;
	unsigned m_MidiKeyNumber;
	GOrguePipe* m_Reference;
	unsigned m_ReferenceID;
	GOrguePipeConfigNode m_PipeConfig;

	virtual void Change(unsigned velocity, unsigned old_velocity);

	void Initialize();
	void LoadData();
	bool LoadCache(GOrgueCache& cache);
	bool SaveCache(GOrgueCacheWriter& cache);
	void UpdateHash(SHA_CTX& ctx);
	const wxString& GetLoadTitle();

	void SetTremulant(bool on);

	void UpdateAmplitude();
	void UpdateTuning();
	void UpdateAudioGroup();

public:
	GOrguePipe(GrandOrgueFile* organfile, GOrgueRank* m_Rank, unsigned midi_key_number);
	virtual ~GOrguePipe();
	virtual void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void Set(unsigned velocity, unsigned referenceID = 0);
	bool InitializeReference();
	unsigned RegisterReference(GOrguePipe* pipe);
	virtual void Abort();
	virtual void PreparePlayback();
	wxString GetFilename();
	bool IsReference();
	GOrguePipeConfigNode& GetPipeConfig();
	virtual void SetTemperament(const GOrgueTemperament& temperament);
	unsigned GetMidiKeyNumber();
};

#endif
