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

#include <wx/docview.h>
#include "IniFileConfig.h"
#include "GOrgueCoupler.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgue.h"
#include "GrandOrgueFile.h"
#include "GrandOrgueFrame.h"
#include "SettingsDialog.h"
#include "GOrgueMidi.h"

extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

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

void GOrgueDivisional::Load(IniFileConfig& cfg, wxString group, int manualNumber, int divisionalNumber, GOrgueDisplayMetrics* displayMetrics)
{

	int i, j, k;
	wxString buffer;

	m_DivisionalNumber = divisionalNumber;
	m_ManualNumber = manualNumber;

	GOrgueManual* associatedManual = organfile->GetManual(m_ManualNumber);

	NumberOfStops = cfg.ReadInteger(group, wxT("NumberOfStops"), 0, associatedManual->GetStopCount());
	NumberOfCouplers = cfg.ReadInteger(group, wxT("NumberOfCouplers"), 0, associatedManual->GetCouplerCount(), organfile->DivisionalsStoreIntermanualCouplers() || organfile->DivisionalsStoreIntramanualCouplers());
	NumberOfTremulants = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, organfile->GetTremulantCount(), organfile->DivisionalsStoreTremulants());

	for (i = 0; i < NumberOfStops; i++)
	{
		buffer.Printf(wxT("Stop%03d"), i + 1);
		j = cfg.ReadInteger( group, buffer, -associatedManual->GetStopCount(), associatedManual->GetStopCount());
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(stop, k, 0, true);
			SET_BIT(stop, k, 1, j >= 0);
		}
	}

	if (organfile->DivisionalsStoreIntermanualCouplers() || organfile->DivisionalsStoreIntramanualCouplers())
	{
		for (i = 0; i < NumberOfCouplers; i++)
		{
			buffer.Printf(wxT("Coupler%03d"), i + 1);
			j = cfg.ReadInteger( group, buffer, -associatedManual->GetCouplerCount(), associatedManual->GetCouplerCount());
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(coupler, k, 0, true);
				SET_BIT(coupler, k, 1, j >= 0);
			}
		}
	}

	if (organfile->DivisionalsStoreTremulants())
	{
		for (i = 0; i < NumberOfTremulants; i++)
		{
			buffer.Printf(wxT("Tremulant%03d"), i + 1);
			j = cfg.ReadInteger( group, buffer, -associatedManual->GetTremulantCount(), associatedManual->GetTremulantCount());
			k = abs(j) - 1;
			if (k >= 0)
			{
				SET_BIT(tremulant, k, 0, true);
				SET_BIT(tremulant, k, 1, j >= 0);
			}
		}
	}

	GOrguePushbutton::Load(cfg, group, displayMetrics);

}

void GOrgueDivisional::Save(IniFileConfig& cfg, bool prefix, wxString group)
{

	int i, k;
	wxString buffer;

	GOrguePushbutton::Save(cfg, prefix, group);

	cfg.SaveHelper(prefix, group, wxT("NumberOfStops"), NumberOfStops);
	cfg.SaveHelper(prefix, group, wxT("NumberOfCouplers"), NumberOfCouplers);
	cfg.SaveHelper(prefix, group, wxT("NumberOfTremulants"), NumberOfTremulants);

	GOrgueManual* associatedManual = organfile->GetManual(m_ManualNumber);

	for (i = 0, k = 1; i < associatedManual->GetStopCount(); i++)
    {
		if (GET_BIT(stop, i, 0))
        {
			buffer.Printf(wxT("Stop%03d"), k++);
			cfg.SaveHelper( prefix, group, buffer, GET_BIT(stop, i, 1) ? i + 1 : -1 - i, true);
        }
    }

	for (i = 0, k = 1; i < associatedManual->GetCouplerCount(); i++)
    {
		if (GET_BIT(coupler, i, 0))
        {
			buffer.Printf(wxT("Coupler%03d"), k++);
			cfg.SaveHelper( prefix, group, buffer, GET_BIT(coupler, i, 1) ? i + 1 : -1 - i, true);
        }
    }

	for (i = 0, k = 1; i < associatedManual->GetTremulantCount(); i++)
    {
		if (GET_BIT(tremulant, i, 0))
        {
			buffer.Printf(wxT("Tremulant%03d"), k++);
			cfg.SaveHelper( prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
        }
    }

}

void GOrgueDivisional::Push(int depth)
{

	int i, k;
	GOrgueManual* associatedManual = organfile->GetManual(m_ManualNumber);

	if (g_sound->GetMidi().SetterActive())
	{
		NumberOfStops = NumberOfCouplers = NumberOfTremulants = 0;
		memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant));
		for (i = 0; i < associatedManual->GetStopCount(); i++)
		{
			if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetStop(i)->Displayed)
				continue;
			NumberOfStops++;
			SET_BIT(stop, i, 0, true);
			SET_BIT(stop, i, 1, associatedManual->GetStop(i)->DefaultToEngaged);
		}
		for (i = 0; i < associatedManual->GetCouplerCount(); i++)
		{
			if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetCoupler(i)->Displayed)
				continue;
			NumberOfCouplers++;
			if ((organfile->DivisionalsStoreIntramanualCouplers() && m_ManualNumber == associatedManual->GetCoupler(i)->DestinationManual) || (organfile->DivisionalsStoreIntermanualCouplers() && m_ManualNumber != associatedManual->GetCoupler(i)->DestinationManual))
			{
				SET_BIT(coupler, i, 0, true);
				SET_BIT(coupler, i, 1, associatedManual->GetCoupler(i)->DefaultToEngaged);
			}
		}
		if (organfile->DivisionalsStoreTremulants())
		{
			for (i = 0; i < associatedManual->GetTremulantCount(); i++)
			{
				if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetTremulant(i)->Displayed)
					continue;
				NumberOfTremulants++;
				SET_BIT(tremulant, i, 0, true);
				SET_BIT(tremulant, i, 1, associatedManual->GetTremulant(i)->DefaultToEngaged);
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
		depth = 1;
	}
	else
	{
		for (k = 0; ;k++)
        {
			for (i = 0; i < associatedManual->GetStopCount(); i++)
				if (GET_BIT(stop, i, 0))
					SET_BIT(stop, i, 1, k < 2 ? associatedManual->GetStop(i)->Set(GET_BIT(stop, i, 1)) : GET_BIT(stop, i, 1));
			for (i = 0; i < associatedManual->GetCouplerCount(); i++)
				if (GET_BIT(coupler, i, 0))
					SET_BIT(coupler, i, 1, k < 2 ? associatedManual->GetCoupler(i)->Set(GET_BIT(coupler, i, 1)) : GET_BIT(coupler, i, 1));
			for (i = 0; i < associatedManual->GetTremulantCount(); i++)
				if (GET_BIT(tremulant, i, 0))
					SET_BIT(tremulant, i, 1, k < 2 ? associatedManual->GetTremulant(i)->Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
			if (k >= 2)
				break;
			organfile->MIDIPretend(!k);
        }
	}

	wxByte used = 0;
	for (i = 0; i < 8; i++)
		used |= stop[i][1];
	for (i = 0; i < 2; i++)
		used |= coupler[i][1] | tremulant[i][1];

	for (k = 0; k < associatedManual->GetDivisionalCount(); k++)
	{
		GOrgueDivisional *divisional = associatedManual->GetDivisional(k);
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

	for (k = 0; k < organfile->GetDivisionalCouplerCount(); k++)
	{
		GOrgueDivisionalCoupler* coupler = organfile->GetDivisionalCoupler(k);
		if (!coupler->DefaultToEngaged)
			continue;
		for (i = 0; i < coupler->NumberOfManuals; i++)
			if (coupler->manual[i] == m_ManualNumber)
				break;
		if (i < coupler->NumberOfManuals)
		{
			for (++i; i < coupler->NumberOfManuals; i++)
				organfile->GetManual(coupler->manual[i])->GetDivisional(m_DivisionalNumber)->Push(depth + 1);
			if (coupler->BiDirectionalCoupling)
			{
				for (i = 0; i < coupler->NumberOfManuals; i++)
				{
					if (coupler->manual[i] == m_ManualNumber)
						break;
					organfile->GetManual(coupler->manual[i])->GetDivisional(m_DivisionalNumber)->Push(depth + 1);
				}
			}
		}
	}

}

