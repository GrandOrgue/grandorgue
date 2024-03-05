/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIOGAUGE_H
#define GOAUDIOGAUGE_H

#include <wx/control.h>
#include <wx/dcmemory.h>

class GOAudioGauge : public wxControl {
private:
  int m_Value;
  bool m_Clip;
  bool m_Update;
  wxMemoryDC m_gaugedc;
  wxBitmap m_gauge;

  void OnPaint(wxPaintEvent &event);
  void Update();

public:
  GOAudioGauge(
    wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition);
  ~GOAudioGauge(void);

  void SetValue(int what);
  void ResetClip();

  DECLARE_EVENT_TABLE()
};

#endif
