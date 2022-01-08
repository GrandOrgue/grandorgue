/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef MIDILIST_H
#define MIDILIST_H

#include <wx/dialog.h>

#include <vector>

#include "GOView.h"

class GOEventDistributor;
class wxButton;
class wxListEvent;
class wxListView;

class MIDIList : public wxDialog, public GOView {
 private:
  wxListView *m_Objects;
  wxButton *m_Edit;
  wxButton *m_Status;
  std::vector<wxButton *> m_Buttons;

  enum {
    ID_LIST = 200,
    ID_EDIT,
    ID_STATUS,
    ID_BUTTON,
    ID_BUTTON_LAST = ID_BUTTON + 2,
  };

  void OnObjectClick(wxListEvent &event);
  void OnObjectDoubleClick(wxListEvent &event);
  void OnEdit(wxCommandEvent &event);
  void OnOK(wxCommandEvent &event);
  void OnStatus(wxCommandEvent &event);
  void OnButton(wxCommandEvent &event);

 public:
  MIDIList(
    GODocumentBase *doc, wxWindow *parent, GOEventDistributor *midi_elements);
  ~MIDIList();

  DECLARE_EVENT_TABLE()
};

#endif
