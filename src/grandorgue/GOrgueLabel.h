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

#ifndef GORGUELABEL_H
#define GORGUELABEL_H

#include "GOrgueMidiConfigurator.h"
#include "GOrgueMidiSender.h"
#include "GOrgueSaveableObject.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GrandOrgueFile;

class GOrgueLabel : private GOrgueSaveableObject, public GOrgueMidiConfigurator
{
protected:
	wxString m_Name;
	wxString m_group;
	GrandOrgueFile* m_organfile;
	GOrgueMidiSender m_sender;

	void Save(GOrgueConfigWriter& cfg);

public:
	GOrgueLabel(GrandOrgueFile* organfile);
	virtual ~GOrgueLabel();
	void Init(GOrgueConfigReader& cfg, wxString group);
	void Load(GOrgueConfigReader& cfg, wxString group);
	const wxString& GetName();
	void SetName(wxString name);
	GOrgueMidiSender& GetMidiSender();
	virtual void Abort();
	virtual void PreparePlayback();
	virtual void PrepareRecording();

	wxString GetMidiType();
	wxString GetMidiName();
	void ShowConfigDialog();
};

#endif
