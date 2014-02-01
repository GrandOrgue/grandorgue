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

#include "GOrgueLabel.h"

#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"

GOrgueLabel::GOrgueLabel(GrandOrgueFile* organfile) :
	m_Name(),
	m_group(),
	m_organfile(organfile),
	m_sender(organfile, MIDI_SEND_LABEL)
{
}

GOrgueLabel::~GOrgueLabel()
{
}

void GOrgueLabel::Init(GOrgueConfigReader& cfg, wxString group)
{
	m_group = group;
	m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOrgueLabel::Load(GOrgueConfigReader& cfg, wxString group)
{
	m_group = group;
	m_sender.Load(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

void GOrgueLabel::Save(GOrgueConfigWriter& cfg)
{
	m_sender.Save(cfg, m_group, m_organfile->GetSettings().GetMidiMap());
}

const wxString& GOrgueLabel::GetName()
{
	return m_Name;
}

void GOrgueLabel::SetName(wxString name)
{
	m_Name = name;
	m_sender.SetLabel(m_Name);
	m_organfile->ControlChanged(this);
}

void GOrgueLabel::Abort()
{
	m_sender.SetLabel(wxEmptyString);
}

void GOrgueLabel::PreparePlayback()
{
	m_sender.SetLabel(m_Name);
}

GOrgueMidiSender& GOrgueLabel::GetMidiSender()
{
	return m_sender;
}
