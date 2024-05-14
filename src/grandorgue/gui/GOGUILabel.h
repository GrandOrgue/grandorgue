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
    unsigned w,
    unsigned h,
    wxString imageFileName,
    unsigned imageNum,
    const wxString &imageMaskFileneme);
  void InitFont(const wxString &fontName, unsigned fontSize);

public:
  GOGUILabel(GOGUIPanel *panel, GOLabelControl *label);
  ~GOGUILabel() override { DestroyBackgroundBitmap(); }

  unsigned GetFontSize() const { return m_Font.GetPoints(); }
  void SetFontSize(unsigned fontSize) { m_Font.SetPoints(fontSize); }
  const wxString &GetFontName() const { return m_Font.GetName(); }
  void SetFontName(const wxString &fontName) { m_Font.SetName(fontName); }
  const wxColour &GetFontColour() { return m_TextColor; }
  void SetFontColour(const wxColour &colour) { m_TextColor = colour; }

  void Init(
    GOConfigReader &cfg,
    wxString group,
    unsigned x_pos = 0,
    unsigned y_pos = 0,
    wxString name = wxT(""),
    unsigned imageno = 1,
    unsigned w = 0,
    unsigned h = 0);
  void Load(GOConfigReader &cfg, wxString group);
  void Layout();

  bool HandleMousePress(int x, int y, bool right, GOGUIMouseState &state);
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
