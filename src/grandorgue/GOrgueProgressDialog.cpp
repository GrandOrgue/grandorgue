/*
 * GrandOrgue - a free pipe organ simulator
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

#include "GOrgueProgressDialog.h"
#include <wx/progdlg.h>

GOrgueProgressDialog::GOrgueProgressDialog() :
	m_dlg(NULL),
	m_last(0),
	m_const(0),
	m_value(0),
	m_max(0)
{
}

GOrgueProgressDialog::~GOrgueProgressDialog()
{
	if (m_dlg)
		m_dlg->Destroy();
}

void GOrgueProgressDialog::Setup(long max, const wxString& title, const wxString& msg)
{
	if (m_dlg)
		m_dlg->Destroy();
	m_dlg = new wxProgressDialog(title, msg, 0xffff, NULL, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
	m_last = 0;
	m_const = 0;
	m_value = 0;
	m_max = 0;
	if (!max)
		max = 1;
	Reset(max, msg);
}

void GOrgueProgressDialog::Reset(long max, const wxString& msg)
{
	m_const += m_value;
	m_max += max;
	m_last--;
	Update(0, msg);
	m_last--;
}

bool GOrgueProgressDialog::Update(unsigned value, const wxString& msg)
{
	if (!m_dlg)
		return true;
	m_value = value;
	if (m_last == wxGetUTCTime())
		return true;
	m_last = wxGetUTCTime();
	if (!m_dlg->Update(0xffff * (m_value + m_const) / m_max, msg))
		return false;
	return true;
}

/*
	wxProgressDialog dlg
		(_("Loading sample set")
		,_("Parsing sample set definition file")
		,32768
		,0
		,wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME
		);
*/
