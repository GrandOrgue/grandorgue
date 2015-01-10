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

#include "GOrgueEventDistributor.h"

#include "GOrgueCacheObject.h"
#include "GOrgueEventHandler.h"
#include "GOrgueSaveableObject.h"

GOrgueEventDistributor::GOrgueEventDistributor() :
	m_handler(),
	m_SaveableObjects(),
	m_MidiConfigurator(),
	m_CacheObjects()
{
}

GOrgueEventDistributor::~GOrgueEventDistributor()
{
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

void GOrgueEventDistributor::UpdateHash(SHA_CTX& ctx)
{
	for (unsigned i = 0; i < m_CacheObjects.size(); i++)
		m_CacheObjects[i]->UpdateHash(ctx);
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
