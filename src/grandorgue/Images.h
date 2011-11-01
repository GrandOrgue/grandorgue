/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GO_IMAGES_H /* using IMAGES_H causes a preprocessor conflict */
#define GO_IMAGES_H

#include <wx/bitmap.h>

typedef wxBitmap (*GetImage_Func)();

wxBitmap GetImage_draw1off();
wxBitmap GetImage_draw1on();
wxBitmap GetImage_draw2off();
wxBitmap GetImage_draw2on();
wxBitmap GetImage_fpiston();
wxBitmap GetImage_fpiston2();
wxBitmap GetImage_gauge();
wxBitmap GetImage_help();
wxBitmap GetImage_hpiston();
wxBitmap GetImage_hpiston2();
wxBitmap GetImage_label();
wxBitmap GetImage_memory();
wxBitmap GetImage_open();
wxBitmap GetImage_panic();
wxBitmap GetImage_polyphony();
wxBitmap GetImage_properties();
wxBitmap GetImage_record();
wxBitmap GetImage_reload();
wxBitmap GetImage_save();
wxBitmap GetImage_set();
wxBitmap GetImage_settings();
wxBitmap GetImage_Splash();
wxBitmap GetImage_transpose();
wxBitmap GetImage_volume();
wxBitmap GetImage_Wood01();
wxBitmap GetImage_Wood01_r();
wxBitmap GetImage_Wood03();
wxBitmap GetImage_Wood03_r();
wxBitmap GetImage_Wood05();
wxBitmap GetImage_Wood05_r();
wxBitmap GetImage_Wood07();
wxBitmap GetImage_Wood07_r();
wxBitmap GetImage_Wood09();
wxBitmap GetImage_Wood09_r();
wxBitmap GetImage_Wood11();
wxBitmap GetImage_Wood11_r();
wxBitmap GetImage_Wood13();
wxBitmap GetImage_Wood13_r();
wxBitmap GetImage_Wood15();
wxBitmap GetImage_Wood15_r();
wxBitmap GetImage_Wood17();
wxBitmap GetImage_Wood17_r();
wxBitmap GetImage_Wood19();
wxBitmap GetImage_Wood19_r();
wxBitmap GetImage_Wood21();
wxBitmap GetImage_Wood21_r();
wxBitmap GetImage_Wood23();
wxBitmap GetImage_Wood23_r();
wxBitmap GetImage_Wood25();
wxBitmap GetImage_Wood25_r();
wxBitmap GetImage_Wood27();
wxBitmap GetImage_Wood27_r();
wxBitmap GetImage_Wood29();
wxBitmap GetImage_Wood29_r();
wxBitmap GetImage_Wood31();
wxBitmap GetImage_Wood31_r();
wxBitmap GetImage_Wood33();
wxBitmap GetImage_Wood33_r();
wxBitmap GetImage_Wood35();
wxBitmap GetImage_Wood35_r();
wxBitmap GetImage_Wood37();
wxBitmap GetImage_Wood37_r();
wxBitmap GetImage_Wood39();
wxBitmap GetImage_Wood39_r();
wxBitmap GetImage_Wood41();
wxBitmap GetImage_Wood41_r();
wxBitmap GetImage_Wood43();
wxBitmap GetImage_Wood43_r();
wxBitmap GetImage_Wood45();
wxBitmap GetImage_Wood45_r();
wxBitmap GetImage_Wood47();
wxBitmap GetImage_Wood47_r();
wxBitmap GetImage_Wood49();
wxBitmap GetImage_Wood49_r();
wxBitmap GetImage_Wood51();
wxBitmap GetImage_Wood51_r();
wxBitmap GetImage_Wood53();
wxBitmap GetImage_Wood53_r();
wxBitmap GetImage_Wood55();
wxBitmap GetImage_Wood55_r();
wxBitmap GetImage_Wood57();
wxBitmap GetImage_Wood57_r();
wxBitmap GetImage_Wood59();
wxBitmap GetImage_Wood59_r();
wxBitmap GetImage_Wood61();
wxBitmap GetImage_Wood61_r();
wxBitmap GetImage_Wood63();
wxBitmap GetImage_Wood63_r();

unsigned GetImageCount_Stop();
wxBitmap GetImage_Stop(unsigned index);
unsigned GetImageCount_Wood();
wxBitmap GetImage_Wood(unsigned index);

#endif
