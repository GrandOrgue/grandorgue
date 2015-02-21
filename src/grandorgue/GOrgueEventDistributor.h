/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2015 GrandOrgue contributors (see AUTHORS)
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

#include "contrib/sha1.h"
#include <vector>

class GOrgueCacheObject;
class GOrgueConfigReader;
class GOrgueConfigWriter;
class GOrgueEventHandler;
class GOrgueMidiConfigurator;
class GOrgueMidiEvent;
class GOrguePlaybackStateHandler;
class GOrgueSaveableObject;

class GOrgueEventDistributor
{
private:
	std::vector<GOrgueEventHandler*> m_handler;
	std::vector<GOrguePlaybackStateHandler*> m_PlaybackStateHandler;
	std::vector<GOrgueSaveableObject*> m_SaveableObjects;
	std::vector<GOrgueMidiConfigurator*> m_MidiConfigurator;
	std::vector<GOrgueCacheObject*> m_CacheObjects;

protected:
	void SendMidi(const GOrgueMidiEvent& event);

	void ReadCombinations(GOrgueConfigReader& cfg);
	void Save(GOrgueConfigWriter& cfg);

	void ResolveReferences();
	void UpdateHash(SHA_CTX& ctx);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOrgueEventDistributor();
	~GOrgueEventDistributor();

	void RegisterEventHandler(GOrgueEventHandler* handler);
	void RegisterPlaybackStateHandler(GOrguePlaybackStateHandler* handler);
	void RegisterCacheObject(GOrgueCacheObject* obj);
	void RegisterSaveableObject(GOrgueSaveableObject* obj);
	void RegisterMidiConfigurator(GOrgueMidiConfigurator* obj);

	unsigned GetMidiConfiguratorCount();
	GOrgueMidiConfigurator* GetMidiConfigurator(unsigned index);

	void HandleKey(int key);

	unsigned GetCacheObjectCount();
	GOrgueCacheObject* GetCacheObject(unsigned index);
};

#endif
