/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2011 GrandOrgue contributors (see AUTHORS)
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
 * MA 02111-1307, USA.
 */

#ifndef GOSOUNDTHREAD_H_
#define GOSOUNDTHREAD_H_

#include <wx/wx.h>
#include "GrandOrgueDef.h"

class GOSoundEngine;

class GOSoundThread : public wxThread
{
private:
	int m_MinSamplerID;
	int m_MaxSamplerID;
	GOSoundEngine* m_Engine;
	unsigned m_FrameCount;
	bool m_Stop;

	wxMutex m_Mutex;
	wxCondition m_Condition;

	void* Entry() STACK_REALIGN;

public:
	GOSoundThread(GOSoundEngine* engine, int min_sampler_id, int max_sampler_id, unsigned n_frames);

	void Delete();
	void Wakeup();
};

#endif
