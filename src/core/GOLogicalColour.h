/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLOGICALCOLOUR_H
#define GOLOGICALCOLOUR_H

struct GOLogicalColour {
  unsigned char m_red;
  unsigned char m_green;
  unsigned char m_blue;

  GOLogicalColour(unsigned char red, unsigned char green, unsigned char blue)
    : m_red(red), m_green(green), m_blue(blue) {}

  GOLogicalColour() : m_red(0), m_green(0), m_blue(0) {}

  static const GOLogicalColour BLACK;
  static const GOLogicalColour BLUE;
  static const GOLogicalColour DARK_BLUE;
  static const GOLogicalColour GREEN;
  static const GOLogicalColour DARK_GREEN;
  static const GOLogicalColour CYAN;
  static const GOLogicalColour DARK_CYAN;
  static const GOLogicalColour RED;
  static const GOLogicalColour DARK_RED;
  static const GOLogicalColour MAGENTA;
  static const GOLogicalColour DARK_MAGENTA;
  static const GOLogicalColour YELLOW;
  static const GOLogicalColour DARK_YELLOW;
  static const GOLogicalColour LIGHT_GREY;
  static const GOLogicalColour DARK_GREY;
  static const GOLogicalColour WHITE;
  static const GOLogicalColour BROWN;
};

#endif /* GOLOGICALCOLOUR_H */
