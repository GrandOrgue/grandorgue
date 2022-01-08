/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef WXGAUGEAUDIO_H
#define WXGAUGEAUDIO_H

#include <wx/control.h>
#include <wx/dcmemory.h>

class wxGaugeAudio : public wxControl {
 private:
  int m_Value;
  bool m_Clip;
  bool m_Update;
  wxMemoryDC m_gaugedc;
  wxBitmap m_gauge;

  void OnPaint(wxPaintEvent& event);
  void Update();

 public:
  wxGaugeAudio(wxWindow* parent, wxWindowID id,
               const wxPoint& pos = wxDefaultPosition);
  ~wxGaugeAudio(void);

  void SetValue(int what);
  void ResetClip();

  DECLARE_EVENT_TABLE()
};

#endif
