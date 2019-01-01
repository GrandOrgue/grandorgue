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

#include "GOrgueDocumentBase.h"

#include "GOrgueView.h"

GOrgueDocumentBase::GOrgueDocumentBase() :
	m_Windows()
{
}

GOrgueDocumentBase::~GOrgueDocumentBase()
{
}

bool GOrgueDocumentBase::WindowExists(WindowType type, void* data)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].type == type && m_Windows[i].data == data)
			return true;
	return false;
}

bool GOrgueDocumentBase::showWindow(WindowType type, void* data)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].type == type && m_Windows[i].data == data)
		{
			m_Windows[i].window->ShowView();
			return true;
		}
	return false;
}

void GOrgueDocumentBase::registerWindow(WindowType type, void* data, GOrgueView *window)
{
	WindowInfo info;
	info.type = type;
	info.data = data;
	info.window = window;
	m_Windows.push_back(info);
	window->ShowView();
}

void GOrgueDocumentBase::unregisterWindow(GOrgueView* window)
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].window == window)
		{
			m_Windows.erase(m_Windows.begin() + i);
			return;
		}
}

void GOrgueDocumentBase::SyncState()
{
	for(unsigned i = 0; i < m_Windows.size(); i++)
		if (m_Windows[i].data)
			m_Windows[i].window->SyncState();
}

void GOrgueDocumentBase::CloseWindows()
{
	while(m_Windows.size() > 0)
	{
		GOrgueView* wnd = m_Windows[0].window;
		m_Windows.erase(m_Windows.begin());
		wnd->RemoveView();
	}
}
