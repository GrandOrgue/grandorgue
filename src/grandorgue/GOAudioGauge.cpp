/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOAudioGauge.h"

#include <wx/dcclient.h>
#include <wx/image.h>

#include "Images.h"

BEGIN_EVENT_TABLE(GOAudioGauge, wxControl)
EVT_PAINT(GOAudioGauge::OnPaint)
END_EVENT_TABLE()

GOAudioGauge::GOAudioGauge(wxWindow *parent, wxWindowID id, const wxPoint &pos)
  : wxControl(parent, id, pos, wxSize(73, 11), wxNO_BORDER),
    m_Value(0),
    m_Clip(false),
    m_Update(false) {
  m_gauge = wxBitmap(GetImage_gauge());
  m_gaugedc.SelectObject(m_gauge);
}

GOAudioGauge::~GOAudioGauge(void) {}

void GOAudioGauge::OnPaint(wxPaintEvent &event) {
  wxPaintDC dc(this);
  m_Update = false;
  int split = (m_Value + 1) << 1;
  dc.Blit(0, 0, split, 11, &m_gaugedc, 0, 11);
  if (66 - split)
    dc.Blit(split, 0, 66 - split, 11, &m_gaugedc, split, 0);
  dc.Blit(66, 0, 7, 11, &m_gaugedc, 66, m_Clip ? 11 : 0);
}

void GOAudioGauge::SetValue(int what) {
  if (what < 0)
    what = 0;
  if (what > 32) {
    what = 32;
    if (!m_Clip)
      m_Clip = true;
  }
  if (what != m_Value) {
    m_Value = what;
    Update();
  }
}

void GOAudioGauge::ResetClip() {
  if (m_Clip) {
    m_Clip = false;
    Update();
  }
}

void GOAudioGauge::Update() {
  if (m_Update)
    return;
  m_Update = true;
  Refresh(false);
}
