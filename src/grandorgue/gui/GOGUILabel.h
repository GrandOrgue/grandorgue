/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUILABEL_H
#define GOGUILABEL_H

#include <wx/colour.h>

#include "GOBitmap.h"
#include "GOFont.h"
#include "GOGUIControl.h"

class GOLabel;

class GOGUILabel : public GOGUIControl {
 private:
  int m_DispXpos;
  int m_DispYpos;
  GOLabel* m_Label;
  GOBitmap m_Bitmap;
  unsigned m_FontSize;
  wxString m_FontName;
  GOFont m_Font;
  wxString m_Text;
  wxColour m_TextColor;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

 public:
  GOGUILabel(GOGUIPanel* panel, GOLabel* label);
  void Init(GOConfigReader& cfg, wxString group, unsigned x_pos = 0,
            unsigned y_pos = 0, wxString name = wxT(""), unsigned imageno = 1);
  void Load(GOConfigReader& cfg, wxString group);
  void Layout();

  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState& state);
  void PrepareDraw(double scale, GOBitmap* background);
  void Draw(GODC& dc);
};

#endif
