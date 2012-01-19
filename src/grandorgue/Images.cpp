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
 * MA 02111-1307, USA.
 */

#include "Images.h"

static const GetImage_Func ImageLoader_Wood[] = {
	GetImage_Wood01, GetImage_Wood01_r, GetImage_Wood03, GetImage_Wood03_r, GetImage_Wood05, GetImage_Wood05_r, 
	GetImage_Wood07, GetImage_Wood07_r, GetImage_Wood09, GetImage_Wood09_r, GetImage_Wood11, GetImage_Wood11_r, 
	GetImage_Wood13, GetImage_Wood13_r, GetImage_Wood15, GetImage_Wood15_r, GetImage_Wood17, GetImage_Wood17_r, 
	GetImage_Wood19, GetImage_Wood19_r, GetImage_Wood21, GetImage_Wood21_r,
	GetImage_Wood23, GetImage_Wood23_r, GetImage_Wood25, GetImage_Wood25_r, GetImage_Wood27, GetImage_Wood27_r, 
	GetImage_Wood29, GetImage_Wood29_r, GetImage_Wood31, GetImage_Wood31_r, GetImage_Wood33, GetImage_Wood33_r, 
	GetImage_Wood35, GetImage_Wood35_r, GetImage_Wood37, GetImage_Wood37_r, GetImage_Wood39, GetImage_Wood39_r, 
	GetImage_Wood41, GetImage_Wood41_r, GetImage_Wood43, GetImage_Wood43_r, GetImage_Wood45, GetImage_Wood45_r, 
	GetImage_Wood47, GetImage_Wood47_r, GetImage_Wood49, GetImage_Wood49_r, GetImage_Wood51, GetImage_Wood51_r, 
	GetImage_Wood53, GetImage_Wood53_r, GetImage_Wood55, GetImage_Wood55_r, GetImage_Wood57, GetImage_Wood57_r, 
	GetImage_Wood59, GetImage_Wood59_r, GetImage_Wood61, GetImage_Wood61_r, GetImage_Wood63, GetImage_Wood63_r, 
};

unsigned GetImageCount_Wood()
{
	return sizeof(ImageLoader_Wood) / sizeof(ImageLoader_Wood[0]);
}

wxBitmap GetImage_Wood(unsigned index)
{
	return ImageLoader_Wood[index]();
}

