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

#ifndef GORGUEDIVISIONAL_H
#define GORGUEDIVISIONAL_H

#include "GOrguePushbutton.h"
#include "IniFileConfig.h"

class GOrgueDivisional : public GOrguePushbutton
{
public:
  GOrgueDivisional():
	GOrguePushbutton(),
	m_DivisionalNumber(0),
	NumberOfStops(0),
	NumberOfCouplers(0),
	NumberOfTremulants(0),
	stop(),coupler(),tremulant()
  {
	for(int i=0;i<8;i++)
	  {
		stop[i][0]=0;
		stop[i][1]=0;
	  }
	for(int i=0;i<2;i++)
	  {
		coupler[i][0]=0;
		coupler[i][1]=0;
		tremulant[i][0]=0;
		tremulant[i][1]=0;
	  }
  }

	void Load(IniFileConfig& cfg, const char* group);
    void Save(IniFileConfig& cfg, bool prefix, wxString group = "Divisional");
	void Push(int depth = 0);

	wxInt16 m_DivisionalNumber;

	wxInt16 NumberOfStops;
	wxInt16 NumberOfCouplers;
	wxInt16 NumberOfTremulants;
	wxByte stop[8][2];
	wxByte coupler[2][2];
	wxByte tremulant[2][2];
};

#endif
