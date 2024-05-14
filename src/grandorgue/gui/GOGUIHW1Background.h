/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIHW1BACKGROUND_H
#define GOGUIHW1BACKGROUND_H

#include <vector>

#include "primitives/GOBitmap.h"

#include "GOGUIControl.h"

class GOGUIHW1Background : public GOGUIControl {
  class GOBackgroundImage {
  public:
    wxRect rect;
    GOBitmap bmp;

    GOBackgroundImage(wxRect Rect, const GOBitmap &Bmp)
      : rect(Rect), bmp(Bmp) {}
  };

private:
  std::vector<GOBackgroundImage> m_Images;

public:
  GOGUIHW1Background(GOGUIPanel *panel);

  void Init(GOConfigReader &cfg, wxString group);
  void Layout();

  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
