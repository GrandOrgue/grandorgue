/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

#include "GOProgressDialog.h"
#include "gui/primitives/go_gui_utils.h"

#include <wx/progdlg.h>
#include <wx/stopwatch.h>

#define DLG_MAX_VALUE 0x10000

GOProgressDialog::GOProgressDialog()
  : m_dlg(NULL), m_last(0), m_const(0), m_value(0), m_max(0) {}

GOProgressDialog::~GOProgressDialog() {
  if (m_dlg)
    m_dlg->Destroy();
}

void GOProgressDialog::Setup(
  long max, const wxString &title, const wxString &msg) {
  if (m_dlg)
    m_dlg->Destroy();
  m_dlg = new wxProgressDialog(
    title,
    msg,
    DLG_MAX_VALUE,
    NULL,
    wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME
      | wxPD_REMAINING_TIME);
  m_dlg->SetIcon(get_go_icon());
  m_last = 0;
  m_const = 0;
  m_value = 0;
  m_max = 0;
  if (!max)
    max = 1;
  Reset(max, msg);
}

void GOProgressDialog::Reset(long max, const wxString &msg) {
  m_const += m_value;
  m_max += max;
  m_last--;
  Update(0, msg);
  m_last--;
}

bool GOProgressDialog::Update(unsigned value, const wxString &msg) {
  if (!m_dlg)
    return true;
  m_value = value;
  if (m_last == wxGetUTCTime())
    return true;
  m_last = wxGetUTCTime();

  int newValue = (DLG_MAX_VALUE - 1) * (m_value + m_const) / m_max;

  if (newValue <= DLG_MAX_VALUE && !m_dlg->Update(newValue, msg))
    return false;
  return true;
}
