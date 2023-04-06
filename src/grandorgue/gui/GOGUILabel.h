/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUILABEL_H
#define GOGUILABEL_H

#include <wx/colour.h>

#include "primitives/GOFont.h"

#include "GOGUIControl.h"

class GOBitmap;
class GOLabelControl;

class GOGUILabel : public GOGUIControl {
private:
  int m_DispXpos;
  int m_DispYpos;
  GOLabelControl *m_Label;
  GOBitmap *m_PBackgroundBitmap; // if nullptr then the label is transparent
  unsigned m_FontSize;
  wxString m_FontName;
  GOFont m_Font;
  wxString m_Text;
  wxColour m_TextColor;
  wxRect m_TextRect;
  unsigned m_TextWidth;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

  void DestroyBackgroundBitmap();
  void InitBackgroundBitmap(
    unsigned x,
    unsigned y,
    wxString imageFileName,
    unsigned imageNum,
    const wxString &imageMaskFileneme);

public:
  GOGUILabel(GOGUIPanel *panel, GOLabelControl *label);
  ~GOGUILabel() override { DestroyBackgroundBitmap(); }
  void Init(
    GOConfigReader &cfg,
    wxString group,
    unsigned x_pos = 0,
    unsigned y_pos = 0,
    wxString name = wxT(""),
    unsigned imageno = 1);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
