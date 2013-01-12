/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2012 GrandOrgue contributors (see AUTHORS)
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
wxBitmap GetImage_reverb();
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
wxBitmap GetImage_Enclosure0();
wxBitmap GetImage_Enclosure1();
wxBitmap GetImage_Enclosure2();
wxBitmap GetImage_Enclosure3();
wxBitmap GetImage_Enclosure4();
wxBitmap GetImage_Enclosure5();
wxBitmap GetImage_Enclosure6();
wxBitmap GetImage_Enclosure7();
wxBitmap GetImage_Enclosure8();
wxBitmap GetImage_Enclosure9();
wxBitmap GetImage_Enclosure10();
wxBitmap GetImage_Enclosure11();
wxBitmap GetImage_Enclosure12();
wxBitmap GetImage_Enclosure13();
wxBitmap GetImage_Enclosure14();
wxBitmap GetImage_Enclosure15();

wxBitmap GetImage_ManualCBlackDown();
wxBitmap GetImage_ManualCBlackUp();
wxBitmap GetImage_ManualCWhiteDown();
wxBitmap GetImage_ManualCWhiteUp();
wxBitmap GetImage_ManualCWoodDown();
wxBitmap GetImage_ManualCWoodUp();
wxBitmap GetImage_ManualDBlackDown();
wxBitmap GetImage_ManualDBlackUp();
wxBitmap GetImage_ManualDWhiteDown();
wxBitmap GetImage_ManualDWhiteUp();
wxBitmap GetImage_ManualDWoodDown();
wxBitmap GetImage_ManualDWoodUp();
wxBitmap GetImage_ManualEBlackDown();
wxBitmap GetImage_ManualEBlackUp();
wxBitmap GetImage_ManualEWhiteDown();
wxBitmap GetImage_ManualEWhiteUp();
wxBitmap GetImage_ManualEWoodDown();
wxBitmap GetImage_ManualEWoodUp();
wxBitmap GetImage_ManualNaturalBlackDown();
wxBitmap GetImage_ManualNaturalBlackUp();
wxBitmap GetImage_ManualNaturalWhiteDown();
wxBitmap GetImage_ManualNaturalWhiteUp();
wxBitmap GetImage_ManualNaturalWoodDown();
wxBitmap GetImage_ManualNaturalWoodUp();
wxBitmap GetImage_ManualSharpBlackDown();
wxBitmap GetImage_ManualSharpBlackUp();
wxBitmap GetImage_ManualSharpWhiteDown();
wxBitmap GetImage_ManualSharpWhiteUp();
wxBitmap GetImage_ManualSharpWoodDown();
wxBitmap GetImage_ManualSharpWoodUp();
wxBitmap GetImage_PedalNaturalBlackDown();
wxBitmap GetImage_PedalNaturalBlackUp();
wxBitmap GetImage_PedalNaturalWoodDown();
wxBitmap GetImage_PedalNaturalWoodUp();
wxBitmap GetImage_PedalSharpBlackDown();
wxBitmap GetImage_PedalSharpBlackUp();
wxBitmap GetImage_PedalSharpWoodDown();
wxBitmap GetImage_PedalSharpWoodUp();


wxBitmap GetImage_GOIcon();

unsigned GetImageCount_Wood();
wxBitmap GetImage_Wood(unsigned index);

#endif
