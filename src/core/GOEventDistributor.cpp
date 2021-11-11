/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOEventDistributor.h"

#include "GOCacheObject.h"
#include "GOControlChangedHandler.h"
#include "GOEventHandler.h"
#include "GOPlaybackStateHandler.h"
#include "GOSaveableObject.h"

GOEventDistributor::GOEventDistributor() :
	m_handler(),
	m_ControlChangedHandler(),
	m_PlaybackStateHandler(),
	m_SaveableObjects(),
	m_MidiConfigurator(),
	m_CacheObjects()
{
}

GOEventDistributor::~GOEventDistributor()
{
}

void GOEventDistributor::Cleanup()
{
	m_ControlChangedHandler.clear();
	m_PlaybackStateHandler.clear();
	m_SaveableObjects.clear();
	m_MidiConfigurator.clear();
	m_CacheObjects.clear();
}

void GOEventDistributor::RegisterEventHandler(GOEventHandler* handler)
{
	m_handler.push_back(handler);
}

void GOEventDistributor::RegisterCacheObject(GOCacheObject* obj)
{
	m_CacheObjects.push_back(obj);
}

void GOEventDistributor::RegisterSaveableObject(GOSaveableObject* obj)
{
	m_SaveableObjects.push_back(obj);
}

void GOEventDistributor::RegisterMidiConfigurator(GOMidiConfigurator* obj)
{
	m_MidiConfigurator.push_back(obj);
}

void GOEventDistributor::RegisterPlaybackStateHandler(GOPlaybackStateHandler* handler)
{
	m_PlaybackStateHandler.push_back(handler);
}

void GOEventDistributor::RegisterControlChangedHandler(GOControlChangedHandler* handler)
{
	m_ControlChangedHandler.push_back(handler);
}

unsigned GOEventDistributor::GetMidiConfiguratorCount()
{
	return m_MidiConfigurator.size();
}

GOMidiConfigurator* GOEventDistributor::GetMidiConfigurator(unsigned index)
{
	return m_MidiConfigurator[index];
}

void GOEventDistributor::SendMidi(const GOMidiEvent& event)
{
	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->ProcessMidi(event);
}

void GOEventDistributor::HandleKey(int key)
{
	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->HandleKey(key);
}

void GOEventDistributor::ReadCombinations(GOConfigReader& cfg)
{
	for(unsigned i = 0; i < m_SaveableObjects.size(); i++)
		m_SaveableObjects[i]->LoadCombination(cfg);
}

void GOEventDistributor::Save(GOConfigWriter& cfg)
{
	for(unsigned i = 0; i < m_SaveableObjects.size(); i++)
		m_SaveableObjects[i]->Save(cfg);
}

void GOEventDistributor::ResolveReferences()
{
	for(unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->Initialize();
}

void GOEventDistributor::UpdateHash(GOHash& hash)
{
	for (unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->UpdateHash(hash);
}

unsigned GOEventDistributor::GetCacheObjectCount()
{
	return m_CacheObjects.size();
}

GOCacheObject* GOEventDistributor::GetCacheObject(unsigned index)
{
	if (index < m_CacheObjects.size())
		return m_CacheObjects[index];
	else
		return NULL;
}

void GOEventDistributor::AbortPlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->AbortPlayback();
}

void GOEventDistributor::PreparePlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->PreparePlayback();
}

void GOEventDistributor::StartPlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->StartPlayback();
}

void GOEventDistributor::PrepareRecording()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->PrepareRecording();
}

void GOEventDistributor::ControlChanged(void* control)
{
	if (!control)
		return;
	for(unsigned i = 0; i < m_ControlChangedHandler.size(); i++)
		m_ControlChangedHandler[i]->ControlChanged(control);
}
