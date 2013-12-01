//
// C++ Interface: GOrgueLCD
//
// Description:
//
//
// Author: Johan Blok,,, <johan@johan-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef GORGUELCD_H
#define GORGUELCD_H

#include <wx/string.h>

#define LCD_DRIVER_IMONVFD 0
#define LCD_DRIVER_MIDI 1

bool GOrgueLCD_Open(const int d=LCD_DRIVER_IMONVFD);
void GOrgueLCD_Close();
void GOrgueLCD_WriteLineOne(const wxString& str);
void GOrgueLCD_WriteLineTwo(const wxString& str, long t=0);
wxString GOrgueLCD_ErrorMsg();

#endif
