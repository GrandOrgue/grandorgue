/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
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

#ifndef GORGUEDOCUMENT_H
#define GORGUEDOCUMENT_H

#include <wx/defs.h>
#include <wx/docview.h>
#include "GOLock.h"

class GOrgueMidiEvent;
class GrandOrgueFile;
class GOrgueSound;

class GOrgueDocument : public wxDocument
{
private:
	GOMutex m_lock;
	bool m_OrganFileReady;
	GrandOrgueFile* m_organfile;
	GOrgueSound& m_sound;

	void CloseOrgan();

	bool OnCloseDocument();
	bool DoOpenDocument(const wxString& file);

public:
	GOrgueDocument(GOrgueSound* sound);
	~GOrgueDocument();

	bool DoOpenDocument(const wxString& file, const wxString& file2);
	bool DoSaveDocument(const wxString& file);

	void OnMidiEvent(GOrgueMidiEvent& event);

	bool Save() { return OnSaveDocument(m_documentFile); }

	GrandOrgueFile* GetOrganFile();
};

#endif
