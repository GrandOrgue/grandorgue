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

#ifndef INIFILECONFIG_H
#define INIFILECONFIG_H

#include <wx/wx.h>

class wxColour;
class wxString;
class wxFileConfig;

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

class IniFileConfig
{
public:
  IniFileConfig(wxFileConfig* iCfg):
	m_cfg(iCfg){}
  bool ReadBoolean(wxString group, wxString key, bool required = true);
  wxColour ReadColor(wxString group, wxString key, bool required = true);
  wxString ReadString(wxString group, wxString key, int nmax = 4096, bool required = true);
  wxInt16 ReadInteger(wxString group, wxString key, int nmin = 0, int nmax = 0, bool required = true);
  wxInt32 ReadLong(wxString group, wxString key, int nmin = 0, int nmax = 0, bool required = true);
  wxInt16 ReadSize(wxString group, wxString key, int nmin = 0, bool required = true);
  wxInt16 ReadFontSize(wxString group, wxString key, bool required = true);
  void SaveHelper(bool prefix, wxString group, wxString key, wxString value);
  void SaveHelper( bool prefix, wxString group, wxString key, int value, bool sign = false, bool force = false);


private:
  bool ReadKey(wxString group, wxString key, void* retval, ValueType type, bool required = true, int nmin = 0, int nmax = 0);
  wxFileConfig* m_cfg;
};

#endif
