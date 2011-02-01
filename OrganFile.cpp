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






void GOrgueWindchest::Load(IniFileConfig& cfg, const char* group)
{
  NumberOfEnclosures=cfg.ReadInteger( group,"NumberOfEnclosures",    0,    6);
  NumberOfTremulants=cfg.ReadInteger( group,"NumberOfTremulants",    0,    6);

  int i;
  char buffer[64];

  for (i = 0; i < NumberOfEnclosures; i++)
	{
	  sprintf(buffer, "Enclosure%03d", i + 1);
	  enclosure[i] = cfg.ReadInteger( group, buffer, 1, organfile->m_NumberOfEnclosures) - 1;
	}
  for (i = 0; i < NumberOfTremulants; i++)
	{
	  sprintf(buffer, "Tremulant%03d", i + 1);
	  tremulant[i] = cfg.ReadInteger( group, buffer, 1, organfile->m_NumberOfTremulants) - 1;
	}
}

bool GOrgueEnclosure::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{
  if (!dc)
	{
	  wxRect rect(m_X, organfile->m_EnclosureY, 46, 61);
	  return rect.Contains(xx, yy);
	}

  dc->SetBrush(*wxBLACK_BRUSH);
  dc->DrawRectangle(m_X, organfile->m_EnclosureY + 13, 46, 44);
  int dx = 1 + ( 3 * MIDIValue) / 127;
  int dy = 1 + (13 * MIDIValue) / 127;
  wxPoint points[4];
  points[0].x = m_X +  7 + dx;
  points[1].x = m_X + 38 - dx;
  points[2].x = m_X + 38 + dx;
  points[3].x = m_X +  7 - dx;
  points[0].y = points[1].y = organfile->m_EnclosureY + 13 + dy;
  points[2].y = points[3].y = organfile->m_EnclosureY + 56 - dy;
  dc->SetBrush(::wxGetApp().frame->m_pedalBrush);
  dc->DrawPolygon(4, points);

  if (dc2)
	dc2->Blit(m_X, organfile->m_EnclosureY + 13, 46, 44, dc, m_X, organfile->m_EnclosureY + 13);
  return false;
}

void GOrgueEnclosure::Load(IniFileConfig& cfg, const char* group)
{
  Name=cfg.ReadString( group,"Name",   64);
  AmpMinimumLevel=cfg.ReadInteger( group,"AmpMinimumLevel",    0,  100);
  MIDIInputNumber=cfg.ReadInteger( group,"MIDIInputNumber",    1,    6);
  Set(127);	// default to full volume until we receive any messages
}

void GOrgueEnclosure::Set(int n)
{
  if (n < 0)
	n = 0;
  if (n > 127)
	n = 127;
  MIDIValue = n;
  wxCommandEvent event(wxEVT_ENCLOSURE, 0);
  event.SetClientData(this);
  ::wxGetApp().frame->AddPendingEvent(event);
}
void GOrgueEnclosure::MIDI(void)
{
  int index = MIDIInputNumber + 1;
  MIDIListenDialog dlg(::wxGetApp().frame, _(s_MIDIMessages[index]), g_sound->i_midiEvents[index], 0);
  if (dlg.ShowModal() == wxID_OK)
	{
	  wxConfigBase::Get()->Write(wxString("MIDI/") + s_MIDIMessages[index], dlg.GetEvent());
	  g_sound->ResetSound();
	}
}




void GOrgueLabel::Load(IniFileConfig& cfg, const char* group)
{
  Name=cfg.ReadString( group,"Name",   64);
  FreeXPlacement=cfg.ReadBoolean( group,"FreeXPlacement");
  FreeYPlacement=cfg.ReadBoolean( group,"FreeYPlacement");

  if (!FreeXPlacement)
	{
	  DispDrawstopCol=cfg.ReadInteger( group,"DispDrawstopCol",    1, organfile->m_DispDrawstopCols);
	  DispSpanDrawstopColToRight=cfg.ReadBoolean( group,"DispSpanDrawstopColToRight");
	}
  else
	DispXpos=cfg.ReadInteger( group,"DispXpos",    0, organfile->m_DispScreenSizeHoriz);

  if (!FreeYPlacement)
	DispAtTopOfDrawstopCol=cfg.ReadBoolean( group,"DispAtTopOfDrawstopCol");
  else
	DispYpos=cfg.ReadInteger( group,"DispYpos",    0, organfile->m_DispScreenSizeVert);

  // NOTICE: this should not be allowed, but some existing definition files use improper values
  if (!DispXpos)
	DispYpos++;
  if (!DispYpos)
	DispYpos++;

  DispLabelColour=cfg.ReadColor( group,"DispLabelColour");
  DispLabelFontSize=cfg.ReadFontSize( group,"DispLabelFontSize");
  DispImageNum=cfg.ReadInteger( group,"DispImageNum",    1,    1);
}



void GOrgueStop::Load(IniFileConfig& cfg, const char* group)
{
  AmplitudeLevel=cfg.ReadInteger( group,"AmplitudeLevel",    0, 1000);
  NumberOfLogicalPipes=cfg.ReadInteger( group,"NumberOfLogicalPipes",    1,  192);
  FirstAccessiblePipeLogicalPipeNumber=cfg.ReadInteger( group,"FirstAccessiblePipeLogicalPipeNumber",    1, NumberOfLogicalPipes);
  FirstAccessiblePipeLogicalKeyNumber=cfg.ReadInteger( group,"FirstAccessiblePipeLogicalKeyNumber",    1,  128);
  NumberOfAccessiblePipes=cfg.ReadInteger( group,"NumberOfAccessiblePipes",    1, NumberOfLogicalPipes);
  WindchestGroup=cfg.ReadInteger( group,"WindchestGroup",    1, organfile->m_NumberOfWindchestGroups);
  Percussive=cfg.ReadBoolean( group,"Percussive");

  WindchestGroup += organfile->m_NumberOfTremulants;    // we would + 1 but it already has it: clever!

  int i;
  char buffer[64];

  pipe = new wxInt16[NumberOfLogicalPipes];
  for (i = 0; i < NumberOfLogicalPipes; i++)
	{
	  sprintf(buffer, "Pipe%03d", i + 1);
	  wxString file = cfg.ReadString( group, buffer);
	  organfile->m_pipe_ptrs.push_back(pipe + i);
	  organfile->m_pipe_files.push_back(file);
	  organfile->m_pipe_windchests.push_back(WindchestGroup);
	  organfile->m_pipe_percussive.push_back(Percussive);
	  if (!file.StartsWith("REF:"))
		{
		  organfile->m_pipe_amplitudes.push_back(organfile->m_AmplitudeLevel * AmplitudeLevel);
		  organfile->m_NumberOfPipes++;
		}
	}

  m_auto = NumberOfLogicalPipes == 1;

  GOrgueDrawstop::Load(cfg, group);
}

bool GOrgueStop::Set(bool on)
{
  if (DefaultToEngaged == on)
	return on;
  DefaultToEngaged = on;
  GOrgueSound::MIDIPretend(true);
  DefaultToEngaged = !on;
  GOrgueSound::MIDIPretend(false);

  bool retval = GOrgueDrawstop::Set(on);

  if (m_auto)
	{
	  GOrgueManual* manual = organfile->m_manual + m_ManualNumber;
	  manual->Set(manual->FirstAccessibleKeyMIDINoteNumber + FirstAccessiblePipeLogicalKeyNumber - 1, on);
	}

  return retval;
}

GOrgueStop::~GOrgueStop(void)
{
  if (pipe)
	delete[] pipe;
}

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

void GOrgueFrameGeneral::Load(IniFileConfig& cfg, const char* group)
{
  m_ManualNumber = -1;

  NumberOfStops=cfg.ReadInteger( group,"NumberOfStops",    0,  448);	// the spec says 512, but 64 * 7 = 448
  NumberOfCouplers=cfg.ReadInteger( group,"NumberOfCouplers",    0,   64);
  NumberOfTremulants=cfg.ReadInteger( group,"NumberOfTremulants",    0,   16);
  NumberOfDivisionalCouplers=cfg.ReadInteger( group,"NumberOfDivisionalCouplers",    0,    8, organfile->m_GeneralsStoreDivisionalCouplers);

  int i, j, k, m;
  char buffer[64];

  for (i = 0; i < NumberOfStops; i++)
	{
	  sprintf(buffer, "StopManual%03d", i + 1);
	  m = cfg.ReadInteger( group, buffer, organfile->m_FirstManual, organfile->m_NumberOfManuals);
	  sprintf(buffer, "StopNumber%03d", i + 1);
	  j = cfg.ReadInteger( group, buffer, -organfile->m_manual[m].NumberOfStops, organfile->m_manual[m].NumberOfStops);
	  k = abs(j) - 1;
	  if (k >= 0)
		{
		  SET_BIT(stop[m], k, 0, true);
		  SET_BIT(stop[m], k, 1, j >= 0);
		}
	}
  for (i = 0; i < NumberOfCouplers; i++)
	{
	  sprintf(buffer, "CouplerManual%03d", i + 1);
	  m = cfg.ReadInteger( group, buffer, organfile->m_FirstManual, organfile->m_NumberOfManuals);
	  sprintf(buffer, "CouplerNumber%03d", i + 1);
	  j = cfg.ReadInteger( group, buffer, -organfile->m_manual[m].NumberOfCouplers, organfile->m_manual[m].NumberOfCouplers);
	  k = abs(j) - 1;
	  if (k >= 0)
		{
		  SET_BIT(coupler[m], k, 0, true);
		  SET_BIT(coupler[m], k, 1, j >= 0);
		}
	}
  for (i = 0; i < NumberOfTremulants; i++)
	{
	  sprintf(buffer, "TremulantNumber%03d", i + 1);
	  j = cfg.ReadInteger( group, buffer, -organfile->m_NumberOfTremulants, organfile->m_NumberOfTremulants);
	  k = abs(j) - 1;
	  if (k >= 0)
		{
		  SET_BIT(tremulant, k, 0, true);
		  SET_BIT(tremulant, k, 1, j >= 0);
		}
	}
  for (i = 0; i < NumberOfDivisionalCouplers; i++)
	{
	  sprintf(buffer, "DivisionalCouplerNumber%03d", i + 1);
	  j = cfg.ReadInteger( group, buffer, -organfile->m_NumberOfDivisionalCouplers, organfile->m_NumberOfDivisionalCouplers);
	  k = abs(j) - 1;
	  if (k >= 0)
		{
		  SET_BIT(divisionalcoupler, k, 0, true);
		  SET_BIT(divisionalcoupler, k, 1, j >= 0);
		}
	}
}

void GOrgueFrameGeneral::Push(int WXUNUSED(depth))
{
  int i, j, k;

  if (g_sound->b_memset)
	{
	  NumberOfStops = NumberOfCouplers = NumberOfTremulants = NumberOfDivisionalCouplers = 0;
	  memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant) + sizeof(divisionalcoupler));
	  for (j = organfile->m_FirstManual; j <= organfile->m_NumberOfManuals; j++)
		{
		  for (i = 0; i < organfile->m_manual[j].NumberOfStops; i++)
			{
			  if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_manual[j].stop[i]->Displayed)
				continue;
			  NumberOfStops++;
			  SET_BIT(stop[j], i, 0, true);
			  SET_BIT(stop[j], i, 1, organfile->m_manual[j].stop[i]->DefaultToEngaged);
			}
		}
	  for (j = organfile->m_FirstManual; j <= organfile->m_NumberOfManuals; j++)
		{
		  for (i = 0; i < organfile->m_manual[j].NumberOfCouplers; i++)
			{
			  if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_manual[j].coupler[i].Displayed)
				continue;
			  NumberOfCouplers++;
			  SET_BIT(coupler[j], i, 0, true);
			  SET_BIT(coupler[j], i, 1, organfile->m_manual[j].coupler[i].DefaultToEngaged);
			}
		}
	  for (i = 0; i < organfile->m_NumberOfTremulants; i++)
		{
		  if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_tremulant[i].Displayed)
			continue;
		  NumberOfTremulants++;
		  SET_BIT(tremulant, i, 0, true);
		  SET_BIT(tremulant, i, 1, organfile->m_tremulant[i].DefaultToEngaged);
		}
	  if (organfile->m_GeneralsStoreDivisionalCouplers)
		{
		  for (i = 0; i < organfile->m_NumberOfDivisionalCouplers; i++)
			{
			  NumberOfDivisionalCouplers++;
			  SET_BIT(divisionalcoupler, i, 0, true);
			  SET_BIT(divisionalcoupler, i, 1, organfile->m_divisionalcoupler[i].DefaultToEngaged);
			}
		}
	  ::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
  else
	{
	  for (k = 0; ;k++)
        {
		  for (j = organfile->m_FirstManual; j <= organfile->m_NumberOfManuals; j++)
            {
			  for (i = 0; i < organfile->m_manual[j].NumberOfStops; i++)
				if (GET_BIT(stop[j], i, 0))
				  SET_BIT(stop[j], i, 1, k < 2 ? organfile->m_manual[j].stop[i]->Set(GET_BIT(stop[j], i, 1)) : GET_BIT(stop[j], i, 1));
			  for (i = 0; i < organfile->m_manual[j].NumberOfCouplers; i++)
				if (GET_BIT(coupler[j], i, 0))
				  SET_BIT(coupler[j], i, 1, k < 2 ? organfile->m_manual[j].coupler[i].Set(GET_BIT(coupler[j], i, 1)) : GET_BIT(coupler[j], i, 1));
            }
		  for (i = 0; i < organfile->m_NumberOfTremulants; i++)
			if (GET_BIT(tremulant, i, 0))
			  SET_BIT(tremulant, i, 1, k < 2 ? organfile->m_tremulant[i].Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
		  for (i = 0; i < organfile->m_NumberOfDivisionalCouplers; i++)
			if (GET_BIT(divisionalcoupler, i, 0))
			  SET_BIT(divisionalcoupler, i, 1, k < 2 ? organfile->m_divisionalcoupler[i].Set(GET_BIT(divisionalcoupler, i, 1)) : GET_BIT(divisionalcoupler, i, 1));
		  if (k > 1)
			break;
		  GOrgueSound::MIDIPretend(!k);
        }
	}

  wxByte used = 0;
  for (j = organfile->m_FirstManual; j <= organfile->m_NumberOfManuals; j++)
    {
	  for (i = 0; i < 8; i++)
		used |= stop[j][i][1];
	  for (i = 0; i < 2; i++)
		used |= coupler[j][i][1];
    }
  used |= tremulant[0][1] | tremulant[1][1] | divisionalcoupler[0][1];

  for (k = 0; k < organfile->m_NumberOfGenerals; k++)
	{
	  GOrgueGeneral *general = organfile->m_general + k;
	  int on = ((general == this && used) ? 2 : 0);
	  if ((general->DispImageNum & 2) != on)
	    {
		  general->DispImageNum = (general->DispImageNum & 1) | on;
		  wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
		  event.SetClientData(general);
		  ::wxGetApp().frame->AddPendingEvent(event);
	    }
	}

  for (j = organfile->m_FirstManual; j <= organfile->m_NumberOfManuals; j++)
    {
	  for (k = 0; k < organfile->m_manual[j].NumberOfDivisionals; k++)
        {
		  GOrgueDivisional *divisional = organfile->m_manual[j].divisional + k;
		  if (divisional->DispImageNum & 2)
            {
			  divisional->DispImageNum &= 1;
			  wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
			  event.SetClientData(divisional);
			  ::wxGetApp().frame->AddPendingEvent(event);
            }
        }
    }
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

void GOrgueFrameGeneral::Save(IniFileConfig& cfg, bool prefix, wxString group)
{
  if (ObjectNumber > 99 && !(NumberOfStops | NumberOfCouplers | NumberOfTremulants | NumberOfDivisionalCouplers))
	return;

  GOrguePushbutton::Save(cfg, prefix, group);

  group.Printf("%s%03d", group.c_str(), ObjectNumber);
  cfg.SaveHelper( prefix, group, "NumberOfStops", NumberOfStops);
  cfg.SaveHelper( prefix, group, "NumberOfCouplers", NumberOfCouplers);
  cfg.SaveHelper( prefix, group, "NumberOfTremulants", NumberOfTremulants);
  cfg.SaveHelper( prefix, group, "NumberOfDivisionalCouplers", NumberOfDivisionalCouplers);

  int i, j, k;
  char buffer[64];

  for (j = organfile->m_FirstManual, k = 1; j <= organfile->m_NumberOfManuals; j++)
	for (i = 0; i < organfile->m_manual[j].NumberOfStops; i++)
	  if (GET_BIT(stop[j], i, 0))
		{
		  sprintf(buffer, "StopManual%03d", k);
		  cfg.SaveHelper( prefix, group, buffer, j, true, true);
		  sprintf(buffer, "StopNumber%03d", k++);
		  cfg.SaveHelper( prefix, group, buffer, GET_BIT(stop[j], i, 1) ? i + 1 : -1 - i, true);
		}
  for (j = organfile->m_FirstManual, k = 1; j <= organfile->m_NumberOfManuals; j++)
	for (i = 0; i < organfile->m_manual[j].NumberOfCouplers; i++)
	  if (GET_BIT(coupler[j], i, 0))
		{
		  sprintf(buffer, "CouplerManual%03d", k);
		  cfg.SaveHelper( prefix, group, buffer, j, true, true);
		  sprintf(buffer, "CouplerNumber%03d", k++);
		  cfg.SaveHelper( prefix, group, buffer, GET_BIT(coupler[j], i, 1) ? i + 1 : -1 - i, true);
		}
  for (i = 0, k = 1; i < organfile->m_NumberOfTremulants; i++)
	if (GET_BIT(tremulant, i, 0))
	  {
		sprintf(buffer, "TremulantNumber%03d", k++);
		cfg.SaveHelper( prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
	  }
  for (i = 0, k = 1; i < organfile->m_NumberOfDivisionalCouplers; i++)
	if (GET_BIT(divisionalcoupler, i, 0))
	  {
		sprintf(buffer, "DivisionalCouplerNumber%03d", k++);
		cfg.SaveHelper( prefix, group, buffer, GET_BIT(divisionalcoupler, i, 1) ? i + 1 : -1 - i, true);
	  }
}

