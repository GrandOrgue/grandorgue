/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTKEYTAB_H
#define GOMIDIEVENTKEYTAB_H

#include <wx/panel.h>

#include "midi/GOMidiShortcutReceiver.h"
#include "modification/GOModificationProxy.h"

class wxChoice;
class wxToggleButton;

class GOMidiEventKeyTab : public wxPanel, public GOModificationProxy {
private:
  GOMidiShortcutReceiver *m_original;
  GOMidiShortcutPattern m_key;
  wxChoice *m_keyselect;
  wxToggleButton *m_listen;
  wxChoice *m_keyminusselect;
  wxToggleButton *m_minuslisten;

  void OnKeyDown(wxKeyEvent &event);
  void OnListenClick(wxCommandEvent &event);
  void OnMinusListenClick(wxCommandEvent &event);

  void Listen(bool enable);
  void FillKeylist(wxChoice *select, unsigned shortcut);

protected:
  enum {
    ID_KEY_SELECT = 200,
    ID_MINUS_KEY_SELECT,
    ID_LISTEN,
    ID_LISTEN_MINUS,
  };

public:
  GOMidiEventKeyTab(wxWindow *parent, GOMidiShortcutReceiver *event);
  ~GOMidiEventKeyTab();

  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
