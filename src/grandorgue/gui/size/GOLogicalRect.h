/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOGICALRECT_H
#define GOLOGICALRECT_H

struct GOLogicalRect {
  int x;
  int y;
  unsigned width;
  unsigned height;

  GOLogicalRect(int xx, int yy, unsigned ww, unsigned hh)
    : x(xx), y(yy), width(ww), height(hh) {}
  GOLogicalRect() : x(0), y(0), width(0), height(0) {}

  bool IsEmpty() const { return width <= 0 || height <= 0; }
};

#endif /* GOLOGICALRECT_H */
