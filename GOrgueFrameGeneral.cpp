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

#include "GOrgueFrameGeneral.h"
#include "GrandOrgueFile.h"
#include "GOrgueSound.h"
#include "GrandOrgue.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

GOrgueFrameGeneral::GOrgueFrameGeneral():
	GOrguePushbutton(),
	NumberOfStops(0),
	NumberOfCouplers(0),
	NumberOfTremulants(0),
	NumberOfDivisionalCouplers(0),
	stop(),
	coupler(),
	tremulant(),
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

