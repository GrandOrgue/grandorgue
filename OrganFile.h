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

#ifndef ORGANFILE_H
#define ORGANFILE_H

#include <vector>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/wx.h>
#include "GOrgueControl.h"
#include "GOrguePushbutton.h"
#include "GOrgueDrawStop.h"
#include "GOrgueCoupler.h"
#include "GOrgueFrameGeneral.h"

#define PHASE_ALIGN_RES 31
#define PHASE_ALIGN_ABS ((PHASE_ALIGN_RES) >> 1)
#define PHASE_ALIGN_RES_VA ((PHASE_ALIGN_RES) * 2)

#define GET_BIT(x,y,z) (x[y >> 3][z] & (0x80 >> (y & 7)) ? true : false)
#define SET_BIT(x,y,z,b) (b ? x[y >> 3][z] |= (0x80 >> (y & 7)) : x[y >> 3][z] &= (0xFFFFFF7F >> (y & 7)))

/* #pragma pack(1) - removed by nappleton */

// TODO: long long on linux or other platforms
#ifdef _WIN32
  typedef __int64 wxInt64;
#endif
#ifdef linux
  typedef long long wxInt64;
#endif

class GOrguePipe;
class IniFileConfig;

class GOrgueSampler
{
public:
	GOrgueSampler* next;		// must be first!
	GOrguePipe* pipe;
	wxByte* ptr;
	int fade, fadein, fadeout, faderemain, fademax, time, offset, type;
	int current, stage, overflowing, shift;
	wxInt64 overflow, f, v;
};

class GOrgueTremulant : public GOrgueDrawstop
{
public:
  GOrgueTremulant():
	GOrgueDrawstop(),
	Period(0),StartRate(0),
	StopRate(0),AmpModDepth(0),
	pipe(NULL)
  {}

	void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix) { GOrgueDrawstop::Save(cfg, prefix, "Tremulant"); }
	bool Set(bool on);

	wxInt32 Period;
	wxInt16 StartRate;
	wxInt16 StopRate;
	wxInt16 AmpModDepth;
	GOrguePipe* pipe;
};

class GOrgueGeneral : public GOrgueFrameGeneral
{
public:
	void Load(IniFileConfig& cfg, const char* group);
	void Save(IniFileConfig& cfg, bool prefix)
	{
        GOrguePushbutton::Save(cfg, prefix, "General");
        GOrgueFrameGeneral::Save(cfg, prefix);
    }
};

class GOrguePiston : public GOrguePushbutton
{
public:
  GOrguePiston():
	GOrguePushbutton(),drawstop(NULL)
  {}
	void Load(IniFileConfig& cfg, const char* group);
	void Save(IniFileConfig& cfg, bool prefix) { GOrguePushbutton::Save(cfg, prefix, "ReversiblePiston"); }
	void Push(int depth = 0);

	GOrgueDrawstop* drawstop;
};

class GOrgueDivisionalCoupler : public GOrgueDrawstop
{
public:
  GOrgueDivisionalCoupler():
	GOrgueDrawstop(),
	BiDirectionalCoupling(false),
	NumberOfManuals(0),manual()
  {
	for (int i = 0; i < 7; ++i)
	{
	  manual[i]=0;
	}
  }

	void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix) { GOrgueDrawstop::Save(cfg, prefix, "DivisionalCoupler"); }
	bool Set(bool on);

	bool BiDirectionalCoupling : 1;
	wxInt16 NumberOfManuals;
	wxInt16 manual[7];
};

/* #pragma pack() - removed by nappleton */

#endif
