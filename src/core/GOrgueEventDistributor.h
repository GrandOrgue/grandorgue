/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
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
