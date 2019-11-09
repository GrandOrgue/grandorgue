/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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

#include "GOSoundTouchWorkItem.h"

#include "GOrgueMemoryPool.h"
#include "mutex_locker.h"

GOSoundTouchWorkItem::GOSoundTouchWorkItem(GOrgueMemoryPool& pool) :
	m_Pool(pool),
	m_Stop(false)
{
}

unsigned GOSoundTouchWorkItem::GetGroup()
{
	return TOUCH;
}

unsigned GOSoundTouchWorkItem::GetCost()
{
	return 0;
}

bool GOSoundTouchWorkItem::GetRepeat()
{
	return false;
}

void GOSoundTouchWorkItem::Run()
{
	GOMutexLocker locker(m_Mutex);
	m_Pool.TouchMemory(m_Stop);
}

void GOSoundTouchWorkItem::Exec()
{
	m_Stop = true;
	GOMutexLocker locker(m_Mutex);
}

void GOSoundTouchWorkItem::Clear()
{
	Reset();
}

void GOSoundTouchWorkItem::Reset()
{
	GOMutexLocker locker(m_Mutex);
	m_Stop = false;
}
