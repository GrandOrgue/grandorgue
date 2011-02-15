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

#include "OrganFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrguePipe.h"
#include "GrandOrgueFile.h"
#include "GOrgueSound.h"
#include "SettingsDialog.h"
#include "OrganDocument.h"
#include <wx/mstream.h>
#include "IniFileConfig.h"
#include "GOrgueDivisional.h"

extern GOrgueSound* g_sound;
extern char* s_MIDIMessages[];
extern GrandOrgueFile* organfile;

void GOrgueTremulant::Load(IniFileConfig& cfg, const char* group)
{
  Period=cfg.ReadLong( group,"Period",   32,  441000);
  StartRate=cfg.ReadInteger( group,"StartRate",    1,  100);
  StopRate=cfg.ReadInteger( group,"StopRate",    1,  100);
  AmpModDepth=cfg.ReadInteger( group,"AmpModDepth",    1,  100);

  GOrgueDrawstop::Load(cfg, group);
}

bool GOrgueTremulant::Set(bool on)
{
  if (DefaultToEngaged == on)
	return on;

  pipe->Set(on);

  return GOrgueDrawstop::Set(on);
}

void GOrgueDivisionalCoupler::Load(IniFileConfig& cfg, const char* group)
{
  BiDirectionalCoupling=cfg.ReadBoolean( group,"BiDirectionalCoupling");
  NumberOfManuals=cfg.ReadInteger( group,"NumberOfManuals",    1,    6);

  int i;
  char buffer[64];

  for (i = 0; i < NumberOfManuals; i++)
	{
	  sprintf(buffer, "Manual%03d", i + 1);
	  manual[i] = cfg.ReadInteger( group, buffer, organfile->m_FirstManual, organfile->m_NumberOfManuals);
	}

  GOrgueDrawstop::Load(cfg, group);
}

bool GOrgueDivisionalCoupler::Set(bool on)
{
  if (DefaultToEngaged == on)
	return on;
  return GOrgueDrawstop::Set(on);
}

void GOrgueGeneral::Load(IniFileConfig& cfg, const char* group)
{
  GOrgueFrameGeneral::Load(cfg, group);
  GOrguePushbutton::Load(cfg, group);
}

void GOrguePiston::Load(IniFileConfig& cfg, const char* group)
{
  int i, j;
  wxString type = cfg.ReadString( group, "ObjectType");
  type.MakeUpper();

  if (type == "STOP")
	{
	  i = cfg.ReadInteger( group, "ManualNumber", organfile->m_FirstManual, organfile->m_NumberOfManuals);
	  j = cfg.ReadInteger( group, "ObjectNumber", 1, organfile->m_manual[i].NumberOfStops) - 1;
	  drawstop = organfile->m_manual[i].stop[j];
	}
  if (type == "COUPLER")
	{
	  i = cfg.ReadInteger( group, "ManualNumber", organfile->m_FirstManual, organfile->m_NumberOfManuals);
	  j = cfg.ReadInteger( group, "ObjectNumber", 1, organfile->m_manual[i].NumberOfCouplers) - 1;
	  drawstop = &organfile->m_manual[i].coupler[j];
	}
  if (type == "TREMULANT")
	{
	  j = ObjectNumber=cfg.ReadInteger( group, "ObjectNumber", 1, organfile->m_NumberOfTremulants) - 1;
	  drawstop = &organfile->m_tremulant[j];
	}

  GOrguePushbutton::Load(cfg, group);
  if (drawstop->DefaultToEngaged ^ drawstop->DisplayInInvertedState)
	DispImageNum ^= 2;
}

void GOrguePiston::Push(int WXUNUSED(depth))
{
  this->drawstop->Push();
}




void GOrgueDrawstop::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
  group.Printf("%s%03d", group.c_str(), ObjectNumber);
  cfg.SaveHelper(prefix, group, "DefaultToEngaged", DefaultToEngaged ? "Y" : "N");
  cfg.SaveHelper(prefix, group, "StopControlMIDIKeyNumber", StopControlMIDIKeyNumber);
}

void GOrguePushbutton::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
  group.Printf("%s%03d", group.c_str(), ObjectNumber);
  cfg.SaveHelper(prefix, group, "MIDIProgramChangeNumber", MIDIProgramChangeNumber);
}

void GOrgueDivisional::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
  GOrguePushbutton::Save(cfg, prefix, group);

  group.Printf("%s%03d", group.c_str(), ObjectNumber);
  cfg.SaveHelper(prefix, group, "NumberOfStops", NumberOfStops);
  cfg.SaveHelper(prefix, group, "NumberOfCouplers", NumberOfCouplers);
  cfg.SaveHelper(prefix, group, "NumberOfTremulants", NumberOfTremulants);

  int i, k;
  char buffer[64];

  for (i = 0, k = 1; i < organfile->m_manual[m_ManualNumber].NumberOfStops; i++)
    {
	  if (GET_BIT(stop, i, 0))
        {
		  sprintf(buffer, "Stop%03d", k++);
		  cfg.SaveHelper( prefix, group, buffer, GET_BIT(stop, i, 1) ? i + 1 : -1 - i, true);
        }
    }
  for (i = 0, k = 1; i < organfile->m_manual[m_ManualNumber].NumberOfCouplers; i++)
    {
	  if (GET_BIT(coupler, i, 0))
        {
		  sprintf(buffer, "Coupler%03d", k++);
		  cfg.SaveHelper( prefix, group, buffer, GET_BIT(coupler, i, 1) ? i + 1 : -1 - i, true);
        }
    }
  for (i = 0, k = 1; i < organfile->m_manual[m_ManualNumber].NumberOfTremulants; i++)
    {
	  if (GET_BIT(tremulant, i, 0))
        {
		  sprintf(buffer, "Tremulant%03d", k++);
		  cfg.SaveHelper( prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
        }
    }
}

