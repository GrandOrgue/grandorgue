/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIENCLOSURE_H
#define GOGUIENCLOSURE_H

#include <wx/colour.h>

#include <vector>

#include "primitives/GOBitmap.h"
#include "primitives/GOFont.h"

#include "GOGUIControl.h"

class GOEnclosure;

class GOGUIEnclosure : public GOGUIControl {
private:
  GOEnclosure *m_enclosure;
  unsigned m_FontSize;
  wxString m_FontName;
  GOFont m_Font;
  wxColor m_TextColor;
  wxString m_Text;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  wxRect m_MouseRect;
  int m_MouseAxisStart;
  int m_MouseAxisEnd;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;
  std::vector<GOBitmap> m_Bitmaps;

public:
  GOGUIEnclosure(GOGUIPanel *panel, GOEnclosure *control);

  void Init(GOConfigReader &cfg, wxString group);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
  bool HandleMouseScroll(int x, int y, int amount);
};

#endif
