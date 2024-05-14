/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIBUTTON_H
#define GOGUIBUTTON_H

#include <wx/colour.h>

#include "primitives/GOBitmap.h"
#include "primitives/GOFont.h"

#include "GOGUIControl.h"

class GOButtonControl;

class GOGUIButton : public GOGUIControl {
protected:
  bool m_IsPiston;
  bool m_DispKeyLabelOnLeft;
  GOButtonControl *m_ButtonControl;
  wxRect m_MouseRect;
  unsigned m_Radius;
  GOBitmap m_OnBitmap;
  GOBitmap m_OffBitmap;
  unsigned m_FontSize;
  wxString m_FontName;
  GOFont m_Font;
  wxColor m_TextColor;
  wxString m_Text;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  unsigned m_DispCol;
  unsigned m_DispRow;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

public:
  GOGUIButton(
    GOGUIPanel *panel, GOButtonControl *control, bool is_piston = false);

  void Init(
    GOConfigReader &cfg,
    wxString group,
    unsigned x_pos,
    unsigned y_pos,
    unsigned image = 0);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
