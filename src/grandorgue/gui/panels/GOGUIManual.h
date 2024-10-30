/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIMANUAL_H
#define GOGUIMANUAL_H

#include <vector>

#include "primitives/GOBitmap.h"

#include "GOGUIControl.h"

class GOManual;

class GOGUIManual : public GOGUIControl {
private:
  typedef struct {
    unsigned MidiNumber;
    bool IsSharp;
    wxRect Rect;
    wxRect MouseRect;
    GOBitmap OnBitmap;
    GOBitmap OffBitmap;
  } KeyInfo;

  GOManual *m_manual;
  unsigned m_ManualNumber;
  std::vector<KeyInfo> m_Keys;
  wxPoint m_LeftTop;

public:
  GOGUIManual(GOGUIPanel *panel, GOManual *manual, unsigned manual_number);

  bool IsSharp(unsigned key);
  unsigned GetKeyCount();

  void Init(GOConfigReader &cfg, wxString group);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
};

#endif
