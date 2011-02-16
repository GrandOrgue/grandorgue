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

#include "GOrgueDivisional.h"
#include "IniFileConfig.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GrandOrgueFrame.h"
#include "GOrgueSound.h"
#include "OrganFile.h"
#include "SettingsDialog.h"
#include <wx/docview.h>
extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;
extern const char* s_MIDIMessages[];

#define GET_BIT(x,y,z) (x[y >> 3][z] & (0x80 >> (y & 7)) ? true : false)
#define SET_BIT(x,y,z,b) (b ? x[y >> 3][z] |= (0x80 >> (y & 7)) : x[y >> 3][z] &= (0xFFFFFF7F >> (y & 7)))

GOrgueDivisional::GOrgueDivisional() :
	GOrguePushbutton(),
	m_DivisionalNumber(0),
	NumberOfStops(0),
	NumberOfCouplers(0),
	NumberOfTremulants(0),
	stop(),
	coupler(),
	tremulant()
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

void GOrgueDivisional::Load(IniFileConfig& cfg, const char* group)
{

	int i, j, k;
	char buffer[64];

	NumberOfStops=cfg.ReadInteger( group,"NumberOfStops",    0, organfile->m_manual[m_ManualNumber].NumberOfStops);
	NumberOfCouplers=cfg.ReadInteger( group,"NumberOfCouplers",    0, organfile->m_manual[m_ManualNumber].NumberOfCouplers, organfile->m_DivisionalsStoreIntermanualCouplers || organfile->m_DivisionalsStoreIntramanualCouplers);
	NumberOfTremulants=cfg.ReadInteger( group,"NumberOfTremulants",    0, organfile->m_NumberOfTremulants, organfile->m_DivisionalsStoreTremulants);

	for (i = 0; i < NumberOfStops; i++)
	{
		sprintf(buffer, "Stop%03d", i + 1);
		j = cfg.ReadInteger( group, buffer, -organfile->m_manual[m_ManualNumber].NumberOfStops, organfile->m_manual[m_ManualNumber].NumberOfStops);
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(stop, k, 0, true);
			SET_BIT(stop, k, 1, j >= 0);
		}
	}

	if (organfile->m_DivisionalsStoreIntermanualCouplers || organfile->m_DivisionalsStoreIntramanualCouplers)
	{
		for (i = 0; i < NumberOfCouplers; i++)
		{
			sprintf(buffer, "Coupler%03d", i + 1);
			j = cfg.ReadInteger( group, buffer, -organfile->m_manual[m_ManualNumber].NumberOfCouplers, organfile->m_manual[m_ManualNumber].NumberOfCouplers);
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(coupler, k, 0, true);
				SET_BIT(coupler, k, 1, j >= 0);
			}
		}
	}

	if (organfile->m_DivisionalsStoreTremulants)
	{
		for (i = 0; i < NumberOfTremulants; i++)
		{
			sprintf(buffer, "Tremulant%03d", i + 1);
			j = cfg.ReadInteger( group, buffer, -organfile->m_manual[m_ManualNumber].NumberOfTremulants, organfile->m_manual[m_ManualNumber].NumberOfTremulants);
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(tremulant, k, 0, true);
				SET_BIT(tremulant, k, 1, j >= 0);
			}
		}
	}

	GOrguePushbutton::Load(cfg, group);

}

void GOrgueDivisional::Save(IniFileConfig& cfg, bool prefix, wxString group)
{

	int i, k;
	char buffer[64];

	GOrguePushbutton::Save(cfg, prefix, group);

	group.Printf("%s%03d", group.c_str(), ObjectNumber);
	cfg.SaveHelper(prefix, group, "NumberOfStops", NumberOfStops);
	cfg.SaveHelper(prefix, group, "NumberOfCouplers", NumberOfCouplers);
	cfg.SaveHelper(prefix, group, "NumberOfTremulants", NumberOfTremulants);

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

void GOrgueDivisional::Push(int depth)
{

	int i, k;

	if (g_sound->b_memset)
	{
		NumberOfStops = NumberOfCouplers = NumberOfTremulants = 0;
		memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant));
		for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfStops; i++)
		{
			if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_manual[m_ManualNumber].stop[i]->Displayed)
				continue;
			NumberOfStops++;
			SET_BIT(stop, i, 0, true);
			SET_BIT(stop, i, 1, organfile->m_manual[m_ManualNumber].stop[i]->DefaultToEngaged);
		}
		for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfCouplers; i++)
		{
			if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_manual[m_ManualNumber].coupler[i].Displayed)
				continue;
			NumberOfCouplers++;
			if ((organfile->m_DivisionalsStoreIntramanualCouplers && m_ManualNumber == organfile->m_manual[m_ManualNumber].coupler[i].DestinationManual) || (organfile->m_DivisionalsStoreIntermanualCouplers && m_ManualNumber != organfile->m_manual[m_ManualNumber].coupler[i].DestinationManual))
			{
				SET_BIT(coupler, i, 0, true);
				SET_BIT(coupler, i, 1, organfile->m_manual[m_ManualNumber].coupler[i].DefaultToEngaged);
			}
		}
		if (organfile->m_DivisionalsStoreTremulants)
		{
			for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfTremulants; i++)
			{
				if (!organfile->m_CombinationsStoreNonDisplayedDrawstops && !organfile->m_tremulant[organfile->m_manual[m_ManualNumber].tremulant[i] - 1].Displayed)
					continue;
				NumberOfTremulants++;
				SET_BIT(tremulant, i, 0, true);
				SET_BIT(tremulant, i, 1, organfile->m_tremulant[organfile->m_manual[m_ManualNumber].tremulant[i] - 1].DefaultToEngaged);
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
		depth = 1;
	}
	else
	{
		for (k = 0; ;k++)
        {
			for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfStops; i++)
				if (GET_BIT(stop, i, 0))
					SET_BIT(stop, i, 1, k < 2 ? organfile->m_manual[m_ManualNumber].stop[i]->Set(GET_BIT(stop, i, 1)) : GET_BIT(stop, i, 1));
			for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfCouplers; i++)
				if (GET_BIT(coupler, i, 0))
					SET_BIT(coupler, i, 1, k < 2 ? organfile->m_manual[m_ManualNumber].coupler[i].Set(GET_BIT(coupler, i, 1)) : GET_BIT(coupler, i, 1));
			for (i = 0; i < organfile->m_manual[m_ManualNumber].NumberOfTremulants; i++)
				if (GET_BIT(tremulant, i, 0))
					SET_BIT(tremulant, i, 1, k < 2 ? organfile->m_tremulant[organfile->m_manual[m_ManualNumber].tremulant[i] - 1].Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
			if (k >= 2)
				break;
			GOrgueSound::MIDIPretend(!k);
        }
	}

	wxByte used = 0;
	for (i = 0; i < 8; i++)
		used |= stop[i][1];
	for (i = 0; i < 2; i++)
		used |= coupler[i][1] | tremulant[i][1];

	for (k = 0; k < organfile->m_manual[m_ManualNumber].NumberOfDivisionals; k++)
	{
		GOrgueDivisional *divisional = organfile->m_manual[m_ManualNumber].divisional + k;
		int on = ((divisional == this && used) ? 2 : 0);
		if ((divisional->DispImageNum & 2) != on)
		{
			divisional->DispImageNum = (divisional->DispImageNum & 1) | on;
			wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
			event.SetClientData(divisional);
			::wxGetApp().frame->AddPendingEvent(event);
	    }
	}

	if (depth)
		return;

	for (k = 0; k < organfile->m_NumberOfDivisionalCouplers; k++)
	{
		if (!organfile->m_divisionalcoupler[k].DefaultToEngaged)
			continue;
		for (i = 0; i < organfile->m_divisionalcoupler[k].NumberOfManuals; i++)
			if (organfile->m_divisionalcoupler[k].manual[i] == m_ManualNumber)
				break;
		if (i < organfile->m_divisionalcoupler[k].NumberOfManuals)
		{
			for (++i; i < organfile->m_divisionalcoupler[k].NumberOfManuals; i++)
				organfile->m_manual[organfile->m_divisionalcoupler[k].manual[i]].divisional[m_DivisionalNumber].Push(depth + 1);
			if (organfile->m_divisionalcoupler[k].BiDirectionalCoupling)
			{
				for (i = 0; i < organfile->m_divisionalcoupler[k].NumberOfManuals; i++)
				{
					if (organfile->m_divisionalcoupler[k].manual[i] == m_ManualNumber)
						break;
					organfile->m_manual[organfile->m_divisionalcoupler[k].manual[i]].divisional[m_DivisionalNumber].Push(depth + 1);
				}
			}
		}
	}

}

