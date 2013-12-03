/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GO_IMAGES_H /* using IMAGES_H causes a preprocessor conflict */
#define GO_IMAGES_H

class wxBitmap;

#define DECLARE_IMAGE(A) wxBitmap GetImage_ ## A ();

DECLARE_IMAGE(gauge);
DECLARE_IMAGE(help);
DECLARE_IMAGE(memory);
DECLARE_IMAGE(open);
DECLARE_IMAGE(panic);
DECLARE_IMAGE(polyphony);
DECLARE_IMAGE(properties);
DECLARE_IMAGE(record);
DECLARE_IMAGE(reload);
DECLARE_IMAGE(save);
DECLARE_IMAGE(set);
DECLARE_IMAGE(settings);
DECLARE_IMAGE(Splash);
DECLARE_IMAGE(transpose);
DECLARE_IMAGE(reverb);
DECLARE_IMAGE(volume);

DECLARE_IMAGE(GOIcon);

#undef DECLARE_IMAGE

#endif
