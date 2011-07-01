//
// C++ Implementation: GOrgueLCD
//
// Description:
//
//
// Author: Johan Blok,,, <johan@johan-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifdef __VFD__

#include "GrandOrgue.h"

#ifdef __WIN32__
#include "imon/SG_VFD.h"
//Define the function prototype
typedef bool (CALLBACK* InitType)(int);
typedef void (CALLBACK* UninitType)();
typedef bool (CALLBACK* SetTextType)(LPCSTR, LPCSTR);

// hold dll handle and pointers to functions in dll
static HINSTANCE dllHandle = NULL;
static InitType iMONVFD_InitPtr = NULL;
static UninitType iMONVFD_UninitPtr = NULL;
static SetTextType iMONVFD_SetTextPtr = NULL;


#endif

static wxTimer timer;
static bool opened=false;
static wxString line1, line2;
static int driver;

bool GOrgueLCD_Open(const int d)
{
	driver=d;
	line1=wxEmptyString;
	line2=wxEmptyString;

	#ifdef __WIN32__
    //Load the SG_VFD dll and keep the handle to it
    dllHandle=LoadLibrary("SG_VFD.dll");
    if (dllHandle==NULL) return false;

    //Get pointer to our function using GetProcAddress:
    iMONVFD_InitPtr=(InitType)GetProcAddress(dllHandle, "iMONVFD_Init");
    iMONVFD_UninitPtr=(UninitType)GetProcAddress(dllHandle, "iMONVFD_Uninit");
    iMONVFD_SetTextPtr=(SetTextType)GetProcAddress(dllHandle, "iMONVFD_SetText");

    // Check whether we got valid function addresses
    if ((iMONVFD_InitPtr==NULL) || (iMONVFD_UninitPtr==NULL) || (iMONVFD_SetTextPtr==NULL)) return false;

    // try to initalize VFD
    opened=iMONVFD_InitPtr(VFDHW_IMON_VFD);
	return opened;
	#endif

	return false;
}

void GOrgueLCD_Close()
{
    #ifdef __WIN32__
    iMONVFD_UninitPtr();
    FreeLibrary(dllHandle);

    dllHandle=NULL;
    iMONVFD_InitPtr=NULL;
    iMONVFD_UninitPtr=NULL;
    iMONVFD_SetTextPtr=NULL;
    #endif
	opened=false;
}

void GOrgueLCD_WriteLineOne(const wxString& str)
{
	if (!opened) return;
	line1=str;
	#ifdef __WIN32__
    iMONVFD_SetTextPtr((LPSTR)(LPCTSTR)line1.c_str(), (LPSTR)(LPCTSTR)line2.c_str());
	#endif
}

void GOrgueLCD_WriteLineTwo(const wxString& str, long t)
{
	if (!opened) return;
	#ifdef __WIN32__
    iMONVFD_SetTextPtr((LPSTR)(LPCTSTR)line1.c_str(), (LPSTR)(LPCTSTR)str.c_str());
	#endif
	if (t>0)
	{ // TODO: temporarily show other message instead of current
//		timer ...

	} else line2=str;

}

void GOrgueLCD_OnTimer()
{
	#ifdef __WIN32__
    iMONVFD_SetTextPtr((LPSTR)(LPCTSTR)line1.c_str(), (LPSTR)(LPCTSTR)line2.c_str());
	#endif
}

wxString GOrgueLCD_ErrorMsg()
{
	return wxEmptyString;
}
#endif
