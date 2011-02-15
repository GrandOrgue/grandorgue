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

#define PHASE_ALIGN_RES 31
#define PHASE_ALIGN_ABS ((PHASE_ALIGN_RES) >> 1)
#define PHASE_ALIGN_RES_VA ((PHASE_ALIGN_RES) * 2)

#define GET_BIT(x,y,z) (x[y >> 3][z] & (0x80 >> (y & 7)) ? true : false)
#define SET_BIT(x,y,z,b) (b ? x[y >> 3][z] |= (0x80 >> (y & 7)) : x[y >> 3][z] &= (0xFFFFFF7F >> (y & 7)))

#pragma pack(1)

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







class GOrgueStop : public GOrgueDrawstop
{
public:
  GOrgueStop():
	GOrgueDrawstop(),m_ManualNumber(0),
	Percussive(false),m_auto(false),
	AmplitudeLevel(0),NumberOfLogicalPipes(0),
	FirstAccessiblePipeLogicalPipeNumber(0),
	FirstAccessiblePipeLogicalKeyNumber(0),
	NumberOfAccessiblePipes(0),	WindchestGroup(0),
	pipe(NULL)
  {}

    void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix) { GOrgueDrawstop::Save(cfg, prefix, "Stop"); }
	bool Set(bool on);
	~GOrgueStop(void);

	wxInt16 m_ManualNumber;

	bool Percussive : 1;
	bool m_auto : 1;
	wxInt16 AmplitudeLevel;
	wxInt16 NumberOfLogicalPipes;
	wxInt16 FirstAccessiblePipeLogicalPipeNumber;
	wxInt16 FirstAccessiblePipeLogicalKeyNumber;
	wxInt16 NumberOfAccessiblePipes;
	wxInt16 WindchestGroup;
	short* pipe;
};

class GOrgueEnclosure : public GOrgueDrawable
{
public:
  GOrgueEnclosure():
	
	m_X(0), AmpMinimumLevel(0),
	MIDIInputNumber(0),
	MIDIValue(0), Name()
  {}

	bool Draw(int xx, int yy, wxDC* dc = 0, wxDC* dc2 = 0);
	void Load(IniFileConfig& cfg, const char* group);
	void Set(int n);
	void MIDI(void);

	wxInt16 m_X;
	wxInt16 AmpMinimumLevel;
	wxInt16 MIDIInputNumber;
	wxInt16 MIDIValue;
	wxString Name;
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

class GOrgueWindchest 
{
public:
  GOrgueWindchest():
	
	m_Volume(0.0),
	NumberOfEnclosures(0),
	NumberOfTremulants(0),
	enclosure(),tremulant()
  {
	for(int i=0;i<6;i++)
	  {
		enclosure[i]=0;
		tremulant[i]=0;
	  }
  }
  
	void Load(IniFileConfig& cfg, const char* group);

	double m_Volume;
	wxInt16 NumberOfEnclosures;
	wxInt16 NumberOfTremulants;
	wxInt16 enclosure[6];
    wxInt16 tremulant[6];
};

class GOrgueFrameGeneral : public GOrguePushbutton
{
public:
  GOrgueFrameGeneral():
	GOrguePushbutton(),
	NumberOfStops(0),
	NumberOfCouplers(0),
	NumberOfTremulants(0),
	NumberOfDivisionalCouplers(0),
	stop(),coupler(),tremulant(),
	divisionalcoupler()
  {
	for(int i=0;i<7;i++)
	  {
		for (int j = 0; i < 8; ++i)
		{
		  stop[i][j][0]=0;
		  stop[i][j][1]=0;
		}
		for (int j = 0; i < 2; ++i)
		{
		  coupler[i][j][0]=0;
		  coupler[i][j][1]=0;
		}
	  }
	tremulant[0][0]=0;
	tremulant[0][1]=0;
	tremulant[1][0]=0;
	tremulant[1][1]=0;
	divisionalcoupler[0][0]=0;
	divisionalcoupler[0][1]=0;
  }

	void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix, wxString group = "General");
	void Push(int depth = 0);

	wxInt16 NumberOfStops;
	wxInt16 NumberOfCouplers;
	wxInt16 NumberOfTremulants;
	wxInt16 NumberOfDivisionalCouplers;

	wxByte stop[7][8][2];
	wxByte coupler[7][2][2];
	wxByte tremulant[2][2];
	wxByte divisionalcoupler[1][2];
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

class GOrgueLabel 
{
public:
  GOrgueLabel():
	FreeXPlacement(false),
	FreeYPlacement(false),
	DispSpanDrawstopColToRight(false),
	DispAtTopOfDrawstopCol(false),
	DispDrawstopCol(0),DispXpos(0),
	DispYpos(0),DispLabelFontSize(0),
	DispImageNum(0),DispLabelColour(0,0,0),
	Name()
  {}

	void Load(IniFileConfig& cfg, const char* group);

	bool FreeXPlacement : 1;
	bool FreeYPlacement : 1;
	bool DispSpanDrawstopColToRight : 1;
	bool DispAtTopOfDrawstopCol : 1;

	wxInt16 DispDrawstopCol;
	wxInt16 DispXpos;
	wxInt16 DispYpos;
	wxInt16 DispLabelFontSize;
	wxInt16 DispImageNum;
	wxColour DispLabelColour;

	wxString Name;
};


#pragma pack()

#endif
