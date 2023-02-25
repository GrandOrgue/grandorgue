/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCHOICE_H
#define GOCHOICE_H

#include <wx/choice.h>

template <class T> class GOChoice : public wxChoice {
public:
  GOChoice(
    wxWindow *parent,
    wxWindowID id,
    const wxPoint &pos = wxDefaultPosition,
    const wxSize &size = wxDefaultSize)
    : wxChoice(parent, id, pos, size) {}

  int Append(const wxString &item, const T value) {
    return wxChoice::Append(item, (void *)value);
  }

  T GetCurrentSelection() {
    return (T)(intptr_t)GetClientData(wxChoice::GetCurrentSelection());
  }

  void SetCurrentSelection(const T value) {
    SetSelection(0);
    for (unsigned i = 0; i < GetCount(); i++)
      if ((void *)value == GetClientData(i))
        SetSelection(i);
  }
};

#endif
