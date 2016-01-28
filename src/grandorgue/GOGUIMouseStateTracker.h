/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

#ifndef GOGUIMOUSESTATETRACKER_H
#define GOGUIMOUSESTATETRACKER_H

#include "GOGUIMouseState.h"
#include <vector>

class GOGUIMouseStateTracker
{
private:
	std::vector<GOGUIMouseState> m_states;

public:

	GOGUIMouseState& GetMouseState()
	{
		return GetState(this);
	}

	GOGUIMouseState& GetState(void * id)
	{
		for(unsigned i = 0; i < m_states.size(); i++)
			if (m_states[i].GetSequence() == id)
				return m_states[i];

		GOGUIMouseState tmp;
		tmp.SetSequence(id);
		m_states.push_back(tmp);
		return m_states[m_states.size() - 1];
	}

	void ReleaseMouseState()
	{
		ReleaseState(this);
	}

	void ReleaseState(void * id)
	{
		for(unsigned i = 0; i < m_states.size(); i++)
			if (m_states[i].GetSequence() == id)
			{
				m_states[i].clear();
				return;
			}
	}
};

#endif
