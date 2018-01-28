/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEEVENTDISTRIBUTOR_H
#define GORGUEEVENTDISTRIBUTOR_H

#include <vector>

class GOrgueCacheObject;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueControlChangedHandler;
class GOrgueEventHandler;
class GOrgueHash;
class GOrgueMidiConfigurator;
class GOrgueMidiEvent;
class GOrguePlaybackStateHandler;
class GOrgueSaveableObject;

class GOrgueEventDistributor
{
private:
	std::vector<GOrgueEventHandler*> m_handler;
	std::vector<GOrgueControlChangedHandler*> m_ControlChangedHandler;
	std::vector<GOrguePlaybackStateHandler*> m_PlaybackStateHandler;
	std::vector<GOrgueSaveableObject*> m_SaveableObjects;
	std::vector<GOrgueMidiConfigurator*> m_MidiConfigurator;
	std::vector<GOrgueCacheObject*> m_CacheObjects;

protected:
	void Cleanup();

	void SendMidi(const GOrgueMidiEvent& event);

	void ReadCombinations(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

	void ResolveReferences();
	void UpdateHash(GOrgueHash& hash);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOrgueEventDistributor();
	~GOrgueEventDistributor();

	void RegisterEventHandler(GOrgueEventHandler* handler);
	void RegisterPlaybackStateHandler(GOrguePlaybackStateHandler* handler);
	void RegisterControlChangedHandler(GOrgueControlChangedHandler* handler);
	void RegisterCacheObject(GOrgueCacheObject* obj);
	void RegisterSaveableObject(GOrgueSaveableObject* obj);
	void RegisterMidiConfigurator(GOrgueMidiConfigurator* obj);

	unsigned GetMidiConfiguratorCount();
	GOrgueMidiConfigurator* GetMidiConfigurator(unsigned index);

	void HandleKey(int key);

	unsigned GetCacheObjectCount();
	GOrgueCacheObject* GetCacheObject(unsigned index);

	void ControlChanged(void* control);
};

#endif
