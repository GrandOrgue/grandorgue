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

#include "GOrgueEventDistributor.h"

#include "GOrgueCacheObject.h"
#include "GOrgueControlChangedHandler.h"
#include "GOrgueEventHandler.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"

GOrgueEventDistributor::GOrgueEventDistributor() :
	m_handler(),
	m_ControlChangedHandler(),
	m_PlaybackStateHandler(),
	m_SaveableObjects(),
	m_MidiConfigurator(),
	m_CacheObjects()
{
}

GOrgueEventDistributor::~GOrgueEventDistributor()
{
}

void GOrgueEventDistributor::Cleanup()
{
	m_ControlChangedHandler.clear();
	m_PlaybackStateHandler.clear();
	m_SaveableObjects.clear();
	m_MidiConfigurator.clear();
	m_CacheObjects.clear();
}

void GOrgueEventDistributor::RegisterEventHandler(GOrgueEventHandler* handler)
{
	m_handler.push_back(handler);
}

void GOrgueEventDistributor::RegisterCacheObject(GOrgueCacheObject* obj)
{
	m_CacheObjects.push_back(obj);
}

void GOrgueEventDistributor::RegisterSaveableObject(GOrgueSaveableObject* obj)
{
	m_SaveableObjects.push_back(obj);
}

void GOrgueEventDistributor::RegisterMidiConfigurator(GOrgueMidiConfigurator* obj)
{
	m_MidiConfigurator.push_back(obj);
}

void GOrgueEventDistributor::RegisterPlaybackStateHandler(GOrguePlaybackStateHandler* handler)
{
	m_PlaybackStateHandler.push_back(handler);
}

void GOrgueEventDistributor::RegisterControlChangedHandler(GOrgueControlChangedHandler* handler)
{
	m_ControlChangedHandler.push_back(handler);
}

unsigned GOrgueEventDistributor::GetMidiConfiguratorCount()
{
	return m_MidiConfigurator.size();
}

GOrgueMidiConfigurator* GOrgueEventDistributor::GetMidiConfigurator(unsigned index)
{
	return m_MidiConfigurator[index];
}

void GOrgueEventDistributor::SendMidi(const GOrgueMidiEvent& event)
{
	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->ProcessMidi(event);
}

void GOrgueEventDistributor::HandleKey(int key)
{
	for(unsigned i = 0; i < m_handler.size(); i++)
		m_handler[i]->HandleKey(key);
}

void GOrgueEventDistributor::ReadCombinations(GOrgueConfigReader& cfg)
{
	for(unsigned i = 0; i < m_SaveableObjects.size(); i++)
		m_SaveableObjects[i]->LoadCombination(cfg);
}

void GOrgueEventDistributor::Save(GOrgueConfigWriter& cfg)
{
	for(unsigned i = 0; i < m_SaveableObjects.size(); i++)
		m_SaveableObjects[i]->Save(cfg);
}

void GOrgueEventDistributor::ResolveReferences()
{
	for(unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->Initialize();
}

void GOrgueEventDistributor::UpdateHash(GOrgueHash& hash)
{
	for (unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->UpdateHash(hash);
}

unsigned GOrgueEventDistributor::GetCacheObjectCount()
{
	return m_CacheObjects.size();
}

GOrgueCacheObject* GOrgueEventDistributor::GetCacheObject(unsigned index)
{
	if (index < m_CacheObjects.size())
		return m_CacheObjects[index];
	else
		return NULL;
}

void GOrgueEventDistributor::AbortPlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->AbortPlayback();
}

void GOrgueEventDistributor::PreparePlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->PreparePlayback();
}

void GOrgueEventDistributor::StartPlayback()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->StartPlayback();
}

void GOrgueEventDistributor::PrepareRecording()
{
	for(unsigned i = 0; i < m_PlaybackStateHandler.size(); i++)
		m_PlaybackStateHandler[i]->PrepareRecording();
}

void GOrgueEventDistributor::ControlChanged(void* control)
{
	if (!control)
		return;
	for(unsigned i = 0; i < m_ControlChangedHandler.size(); i++)
		m_ControlChangedHandler[i]->ControlChanged(control);
}
