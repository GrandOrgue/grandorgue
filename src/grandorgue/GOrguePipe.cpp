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

#include "GOrguePipe.h"

#include "GOrgueRank.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrguePipe::GOrguePipe (GrandOrgueFile* organfile, GOrgueRank* rank, unsigned midi_key_number) :
	m_Velocity(0),
	m_Velocities(1),
	m_organfile(organfile),
	m_Rank(rank),
	m_MidiKeyNumber(midi_key_number)
{
}

GOrguePipe::~GOrguePipe()
{
}

unsigned GOrguePipe::RegisterReference(GOrguePipe* pipe)
{
	unsigned id = m_Velocities.size();
	m_Velocities.resize(id + 1);
	return id;
}

void GOrguePipe::Abort()
{
}

void GOrguePipe::PreparePlayback()
{
	m_Velocity = 0;
	for(unsigned i = 0; i < m_Velocities.size(); i++)
		m_Velocities[i] = 0;
}

void GOrguePipe::SetTemperament(const GOrgueTemperament& temperament)
{
}

void GOrguePipe::Set(unsigned velocity, unsigned referenceID)
{
	if (m_Velocities[referenceID] <= velocity && velocity <= m_Velocity)
	{
		m_Velocities[referenceID] = velocity;
		return;
	}
	unsigned last_velocity = m_Velocity;
	if (velocity >= m_Velocity)
	{
		m_Velocities[referenceID] = velocity;
		m_Velocity = velocity;
	}
	else
	{
		m_Velocities[referenceID] = velocity;
		m_Velocity = m_Velocities[0];
		for(unsigned i = 1; i < m_Velocities.size(); i++)
			if (m_Velocity < m_Velocities[i])
				m_Velocity = m_Velocities[i];
	}

	Change(m_Velocity, last_velocity);
}
