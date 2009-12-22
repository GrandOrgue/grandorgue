//
// C++ Interface: MyLCD
//
// Description:
//
//
// Author: Johan Blok,,, <johan@johan-laptop>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#pragma once

#define LCD_DRIVER_IMONVFD 0
#define LCD_DRIVER_MIDI 1

bool MyLCD_Open(const int d=LCD_DRIVER_IMONVFD);
void MyLCD_Close();
void MyLCD_WriteLineOne(const wxString& str);
void MyLCD_WriteLineTwo(const wxString& str, long t=0);
wxString MyLCD_ErrorMsg();




