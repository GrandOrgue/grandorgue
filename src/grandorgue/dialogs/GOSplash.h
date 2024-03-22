/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSPLASH_H
#define GOSPLASH_H

#include <wx/dialog.h>
#include <wx/timer.h>

#include "common/GODialogCloser.h"

class GOSplashBitmap;

class GOSplash : public wxDialog, private GODialogCloser {
private:
  GOSplashBitmap *m_Image;
  wxTimer m_Timer;
  bool m_hasTimeout;

  GOSplash(bool has_timeout, wxWindow *parent, wxWindowID id);
  ~GOSplash();

  void OnShow() override;
  void OnCloseWindow(wxCloseEvent &event) override;

  void OnNotify(wxTimerEvent &event);

  void DrawText(wxBitmap &bitmap);

public:
  static void DoSplash(bool hasTimeout, wxWindow *parent);

  DECLARE_EVENT_TABLE()
};

#endif
