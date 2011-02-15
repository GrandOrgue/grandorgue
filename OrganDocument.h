/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef ORGANDOCUMENT_H
#define ORGANDOCUMENT_H

#include <wx/defs.h>
#include <wx/docview.h>

class RIFF
{
public:
	void Analyze(void* ptr, unsigned i)
	{
		char* p = (char*)ptr + i;
		type = *((int*)p);
		unsigned* pp = (unsigned*)(p + 4);
		size = wxUINT32_SWAP_ON_BE(*pp);
	}
	unsigned GetSize()
	{
		return size;
	}
	bool operator== (const char* cmp)
	{
	    return type == *((int*)cmp);
	}
private:
    int type;
	unsigned size;
};

class OrganDocument : public wxDocument
{
	DECLARE_DYNAMIC_CLASS(OrganDocument)
public:
	OrganDocument(void) { b_loaded = false; };
	~OrganDocument(void);

	bool OnCloseDocument();
	bool DoOpenDocument(const wxString& file, const wxString& file2);
	bool DoOpenDocument(const wxString& file);
	bool DoSaveDocument(const wxString& file);

    bool Save() { return OnSaveDocument(m_documentFile); }

	bool b_loaded;

private:
	void CloseOrgan();
};

#endif
