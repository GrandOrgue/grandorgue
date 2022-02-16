/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
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
};

#endif /* GOLOGICALCOLOUR_H */
