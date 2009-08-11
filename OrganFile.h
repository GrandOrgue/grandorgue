/*
 * GrandOrgue - Copyright (C) 2009 JL Derouineau - free pipe organ simulator based on MyOrgan Copyright (C) 2006 Kloria Publishing LLC
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

#define PHASE_ALIGN_RES 31
#define PHASE_ALIGN_ABS ((PHASE_ALIGN_RES) >> 1)
#define PHASE_ALIGN_RES_VA ((PHASE_ALIGN_RES) * 2)

enum ValueType
{
	ORGAN_BOOLEAN,
	ORGAN_FONTSIZE,
	ORGAN_SIZE,
	ORGAN_COLOR,
	ORGAN_STRING,
	ORGAN_INTEGER,
	ORGAN_LONG,
};

#pragma pack(1)

// TODO: long long on linux or other platforms
#ifdef _WIN32
  typedef __int64 wxInt64;
#endif
#ifdef linux
  typedef long long wxInt64;
#endif

class MyPipe;

class MyObject
{
public:
	void* operator new (size_t s);
	void* operator new[] (size_t s);
//	virtual ~MyObject(void) { };		We never access MyObject directly, so this shouldn't be necessary.
};

class MySampler
{
public:
	MySampler* next;		// must be first!
	MyPipe* pipe;
	wxByte* ptr;
	int fade, fadein, fadeout, faderemain, fademax, time, offset, type;
	int current, stage, overflowing, shift;
	wxInt64 overflow, f, v;
};

class MyPipe
{
public:
	void Set(bool on);

	unsigned _fourcc;
    unsigned _adler32;
	MyPipe* _this;
	float pitch;
	MySampler* sampler;
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

class MyControl : public MyObject
{
public:
	void Load(wxFileConfig* cfg, const char* group);

	bool Displayed : 1;
	bool DispKeyLabelOnLeft : 1;
	wxInt16 ObjectNumber;
	wxInt16 ShortcutKey;
	wxInt16 DispLabelFontSize;
	wxColour DispLabelColour;
	wxString Name;
};

class MyDrawstop : public MyControl
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix, wxString group);
	bool Draw(int xx, int yy, wxDC* dc = 0, wxDC* dc2 = 0);
	void Push(void) { Set(DefaultToEngaged ^ true); };
	void MIDI(void);
	virtual bool Set(bool on);
	virtual ~MyDrawstop() { };

	bool DefaultToEngaged : 1;
	bool DisplayInInvertedState : 1;
	wxInt16 DispDrawstopRow;
	wxInt16 DispDrawstopCol;
	wxInt16 DispImageNum;
	wxInt16 StopControlMIDIKeyNumber;
};

class MyPushbutton : public MyControl
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix, wxString group);
	bool Draw(int xx, int yy, wxDC* dc = 0, wxDC* dc2 = 0);
	virtual void Push(int depth = 0) { };
	void MIDI(void);
	virtual ~MyPushbutton() { };

	wxInt16 m_ManualNumber;
	wxInt16 DispButtonRow;
	wxInt16 DispButtonCol;
	wxInt16 DispImageNum;
	wxInt16 MIDIProgramChangeNumber;
};

class MyCoupler : public MyDrawstop
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix) { MyDrawstop::Save(cfg, prefix, "Coupler"); }
	bool Set(bool on);

	bool UnisonOff : 1;
	bool CoupleToSubsequentUnisonIntermanualCouplers : 1;
	bool CoupleToSubsequentUpwardIntermanualCouplers : 1;
	bool CoupleToSubsequentDownwardIntermanualCouplers : 1;
	bool CoupleToSubsequentUpwardIntramanualCouplers : 1;
	bool CoupleToSubsequentDownwardIntramanualCouplers : 1;
	wxInt16 DestinationManual;
	wxInt16 DestinationKeyshift;
};

class MyDivisional : public MyPushbutton
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix, wxString group = "Divisional");
	void Push(int depth = 0);

	wxInt16 m_DivisionalNumber;

	wxInt16 NumberOfStops;
	wxInt16 NumberOfCouplers;
	wxInt16 NumberOfTremulants;
	wxByte stop[8][2];
	wxByte coupler[2][2];
	wxByte tremulant[2][2];
};

class MyStop : public MyDrawstop
{
public:
    void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix) { MyDrawstop::Save(cfg, prefix, "Stop"); }
	bool Set(bool on);
	~MyStop(void);

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

class MyManual : public MyObject
{
public:
	void Load(wxFileConfig* cfg, const char* group);
	void Set(int note, bool on, bool pretend = false, int depth = 0, MyCoupler* prev = 0);
	void MIDI(void);
	~MyManual(void);

	bool Displayed : 1;
	bool DispKeyColourInverted : 1;
	bool DispKeyColourWooden : 1;

	wxInt16 m_ManualNumber;
	wxInt16 m_Width, m_Height, m_X, m_Y, m_KeysY, m_PistonY;
	wxInt16 m_MIDI[85];

	wxInt16 NumberOfLogicalKeys;
	wxInt16 FirstAccessibleKeyLogicalKeyNumber;
	wxInt16 FirstAccessibleKeyMIDINoteNumber;
	wxInt16 NumberOfAccessibleKeys;
	wxInt16 MIDIInputNumber;
	wxInt16 NumberOfStops;
	wxInt16 NumberOfCouplers;
	wxInt16 NumberOfDivisionals;
	wxInt16 NumberOfTremulants;
	wxInt16 tremulant[10];

	wxString Name;

	MyStop* stop;
	MyCoupler* coupler;
	MyDivisional* divisional;
};

class MyEnclosure : public MyObject
{
public:
	bool Draw(int xx, int yy, wxDC* dc = 0, wxDC* dc2 = 0);
	void Load(wxFileConfig* cfg, const char* group);
	void Set(int n);
	void MIDI(void);

	wxInt16 m_X;
	wxInt16 AmpMinimumLevel;
	wxInt16 MIDIInputNumber;
	wxInt16 MIDIValue;
	wxString Name;
};

class MyTremulant : public MyDrawstop
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix) { MyDrawstop::Save(cfg, prefix, "Tremulant"); }
	bool Set(bool on);

	wxInt32 Period;
	wxInt16 StartRate;
	wxInt16 StopRate;
	wxInt16 AmpModDepth;
	MyPipe* pipe;
};

class MyWindchest : public MyObject
{
public:
	void Load(wxFileConfig* cfg, const char* group);

	double m_Volume;
	wxInt16 NumberOfEnclosures;
	wxInt16 NumberOfTremulants;
	wxInt16 enclosure[6];
    wxInt16 tremulant[6];
};

class MyFrameGeneral : public MyPushbutton
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix, wxString group = "General");
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

class MyGeneral : public MyFrameGeneral
{
public:
	void Load(wxFileConfig* cfg, const char* group);
	void Save(wxFileConfig& cfg, bool prefix)
	{
        MyPushbutton::Save(cfg, prefix, "General");
        MyFrameGeneral::Save(cfg, prefix);
    }
};

class MyPiston : public MyPushbutton
{
public:
	void Load(wxFileConfig* cfg, const char* group);
	void Save(wxFileConfig& cfg, bool prefix) { MyPushbutton::Save(cfg, prefix, "ReversiblePiston"); }
	void Push(int depth = 0);

	MyDrawstop* drawstop;
};

class MyDivisionalCoupler : public MyDrawstop
{
public:
	void Load(wxFileConfig* cfg, const char* group);
    void Save(wxFileConfig& cfg, bool prefix) { MyDrawstop::Save(cfg, prefix, "DivisionalCoupler"); }
	bool Set(bool on);

	bool BiDirectionalCoupling : 1;
	wxInt16 NumberOfManuals;
	wxInt16 manual[7];
};

class MyLabel : public MyObject
{
public:
	void Load(wxFileConfig* cfg, const char* group);

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

class MyOrganFile : public MyObject
{
public:
	wxString Load(const wxString& file, const wxString& file2 = wxEmptyString);
	void Save(const wxString& file);
	void Revert(wxFileConfig& cfg);
	~MyOrganFile(void);

	void CompressWAV(char*& compress, short* fv, short* ptr, int count, int channels, int stage);

	std::vector<wxString> pipe_filenames;
	std::vector<int> pipe_filesizes;

	std::vector<wxString> pipe_files;
	std::vector<wxInt16*> pipe_ptrs;
	std::vector<wxInt16> pipe_windchests;
	std::vector<wxInt16> pipe_percussive;
	std::vector<int> pipe_amplitudes;
	wxBitmap m_images[9];
	wxFileName m_path;
	int b_squash;
	MyPipe* m_compress_p;
	wxFileConfig *m_cfg;
	wxString m_filename;
	long m_elapsed;

	bool m_opening : 1;
	bool b_customized : 1;

	bool DivisionalsStoreIntermanualCouplers : 1;
	bool DivisionalsStoreIntramanualCouplers : 1;
	bool DivisionalsStoreTremulants : 1;
	bool GeneralsStoreDivisionalCouplers : 1;
	bool CombinationsStoreNonDisplayedDrawstops : 1;
	bool DispDrawstopColsOffset : 1;
	bool DispDrawstopOuterColOffsetUp : 1;
	bool DispPairDrawstopCols : 1;
	bool DispExtraPedalButtonRow : 1;
	bool DispExtraPedalButtonRowOffset : 1;
	bool DispExtraPedalButtonRowOffsetRight : 1;
	bool DispButtonsAboveManuals : 1;
	bool DispTrimAboveManuals : 1;
	bool DispTrimBelowManuals : 1;
	bool DispTrimAboveExtraRows : 1;
	bool DispExtraDrawstopRowsAboveExtraButtonRows : 1;

	wxInt16 m_JambLeftRightWidth, m_JambLeftRightHeight, m_JambLeftRightY;
	wxInt16 m_JambLeftX, m_JambRightX;
	wxInt16 m_JambTopWidth, m_JambTopHeight;
	wxInt16 m_JambTopX, m_JambTopY, m_HackY;
	wxInt16 m_JambTopPiston, m_JambTopDrawstop;
	wxInt16 m_CenterX, m_CenterY, m_CenterWidth;
	wxInt16 m_PistonX, m_PistonWidth, m_PistonTopHeight;
	wxInt16 m_EnclosureWidth, m_EnclosureY;

	wxInt16 HighestSampleFormat;
	wxInt16 FirstManual;
	wxInt16 NumberOfManuals;
	wxInt16 NumberOfEnclosures;
	wxInt16 NumberOfTremulants;
	wxInt16 NumberOfWindchestGroups;
	wxInt16 NumberOfReversiblePistons;
	wxInt16 NumberOfLabels;
	wxInt16 NumberOfGenerals;
	wxInt16 NumberOfFrameGenerals;
	wxInt16 NumberOfDivisionalCouplers;
	wxInt16 NumberOfStops;
	wxInt16 NumberOfPipes;
	wxInt16 AmplitudeLevel;

	wxInt16 DispScreenSizeHoriz;
	wxInt16 DispScreenSizeVert;
	wxInt16 DispDrawstopBackgroundImageNum;
	wxInt16 DispConsoleBackgroundImageNum;
	wxInt16 DispKeyHorizBackgroundImageNum;
	wxInt16 DispKeyVertBackgroundImageNum;
	wxInt16 DispDrawstopInsetBackgroundImageNum;

	wxInt16 DispDrawstopCols;
	wxInt16 DispDrawstopRows;
	wxInt16 DispExtraDrawstopRows;
	wxInt16 DispExtraDrawstopCols;
	wxInt16 DispButtonCols;
	wxInt16 DispExtraButtonRows;

	wxColour DispShortcutKeyLabelColour;

    wxString HauptwerkOrganFileFormatVersion;
	wxString ChurchName;
	wxString ChurchAddress;
	wxString OrganBuilder;
	wxString OrganBuildDate;
	wxString OrganComments;
	wxString RecordingDetails;
    wxString InfoFilename;

	wxString DispControlLabelFont;
	wxString DispShortcutKeyLabelFont;
	wxString DispGroupLabelFont;

	MyManual manual[7];
	MyEnclosure* enclosure;
	MyTremulant* tremulant;
	MyWindchest* windchest;
	MyPiston* piston;
	MyLabel* label;
	MyGeneral* general;
	MyFrameGeneral* framegeneral;
	MyDivisionalCoupler* divisionalcoupler;
	MyPipe** pipe;
};

#pragma pack()
