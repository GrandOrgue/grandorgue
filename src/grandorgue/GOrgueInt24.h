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

#ifndef GORGUEINT24_H
#define GORGUEINT24_H

#include <stdint.h>

#pragma pack(push, 1)

/* GCC generates for int x:24 an extra sign extend compared to the following */ 

typedef struct {
	uint16_t lo, ;
	int8_t hi;

	void operator= (int value)
	{
		lo = (value >> 0) & 0xffff;
		hi = (value >> 16) & 0xff;
	}

	operator int()
	{
		return ((hi << 16) | lo);
	}
} Int24;

#pragma pack(pop)

#endif
