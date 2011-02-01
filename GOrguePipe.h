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

#pragma once
#include <wx/wx.h>

#define PHASE_ALIGN_RES 31
#define PHASE_ALIGN_ABS ((PHASE_ALIGN_RES) >> 1)
#define PHASE_ALIGN_RES_VA ((PHASE_ALIGN_RES) * 2)

class GOrgueSampler;

class GOrguePipe
{
public:

	void Set(bool on);
	int ra_getindex(int *f, int *v);
	int ra_getindex(short *f, short *v);

	unsigned _fourcc;
    unsigned _adler32;
	GOrguePipe* _this;
	float pitch;
	GOrgueSampler* sampler;
	int instances;
	int WindchestGroup;
	wxByte* ptr[3];
	int offset[3];
	int types[3];
	wxInt16 f[3][4];	// v______ joined
	wxInt16 v[3][4];	// ^
	int ra_shift, ra_amp, ra_volume, ra_factor;
	int ra_offset[PHASE_ALIGN_RES_VA];
	wxInt16 ra_f[PHASE_ALIGN_RES_VA][4];
	wxInt16 ra_v[PHASE_ALIGN_RES_VA][4];
	wxByte data[1];	// expandable
};
