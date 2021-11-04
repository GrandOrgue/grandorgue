/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOEVENTDISTRIBUTOR_H
#define GOEVENTDISTRIBUTOR_H

#include <vector>

class GOCacheObject;
class GOConfigReader;
class GOConfigWriter;
class GOControlChangedHandler;
class GOEventHandler;
class GOHash;
class GOMidiConfigurator;
class GOMidiEvent;
class GOPlaybackStateHandler;
class GOSaveableObject;

class GOEventDistributor
{
private:
	std::vector<GOEventHandler*> m_handler;
	std::vector<GOControlChangedHandler*> m_ControlChangedHandler;
	std::vector<GOPlaybackStateHandler*> m_PlaybackStateHandler;
	std::vector<GOSaveableObject*> m_SaveableObjects;
	std::vector<GOMidiConfigurator*> m_MidiConfigurator;
	std::vector<GOCacheObject*> m_CacheObjects;

protected:
	void Cleanup();

	void SendMidi(const GOMidiEvent& event);

	void ReadCombinations(GOConfigReader& cfg);
	void Save(GOConfigWriter& cfg);

	void ResolveReferences();
	void UpdateHash(GOHash& hash);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOEventDistributor();
	~GOEventDistributor();

	void RegisterEventHandler(GOEventHandler* handler);
	void RegisterPlaybackStateHandler(GOPlaybackStateHandler* handler);
	void RegisterControlChangedHandler(GOControlChangedHandler* handler);
	void RegisterCacheObject(GOCacheObject* obj);
	void RegisterSaveableObject(GOSaveableObject* obj);
	void RegisterMidiConfigurator(GOMidiConfigurator* obj);

	unsigned GetMidiConfiguratorCount();
	GOMidiConfigurator* GetMidiConfigurator(unsigned index);

	void HandleKey(int key);

	unsigned GetCacheObjectCount();
	GOCacheObject* GetCacheObject(unsigned index);

	void ControlChanged(void* control);
};

#endif
