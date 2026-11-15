/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOGUIIMAGE_H
#define GOGUIIMAGE_H

#include "primitives/GOBitmap.h"

#include "GOGUIControl.h"

class GOGUIImage : public GOGUIControl {
protected:
  GOBitmap m_Bitmap;
  unsigned m_TileOffsetX;
  unsigned m_TileOffsetY;

public:
  GOGUIImage(GOGUIPanel *panel);

  void Load(GOConfigReader &cfg, wxString group);

  /** Marks ODF panel images as part of the panel background. */
  bool IsBackground() const override { return true; }
  void PrepareDraw(double scale, GOBitmap *background);
  void Draw(GODC &dc);
};

#endif
