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

#include "GOrgueCoupler.h"
#include "GOrgueDivisional.h"
#include "GOrgueDivisionalCoupler.h"
#include "GOrgueGeneral.h"
#include "GOrgueSound.h"
#include "GOrgueStop.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "GOrgueMidi.h"

/* TODO: This should not be... */
extern GrandOrgueFile* organfile;
extern GOrgueSound* g_sound;

#define GET_BIT(x,y,z) (x[y >> 3][z] & (0x80 >> (y & 7)) ? true : false)
#define SET_BIT(x,y,z,b) (b ? x[y >> 3][z] |= (0x80 >> (y & 7)) : x[y >> 3][z] &= (0xFFFFFF7F >> (y & 7)))

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

void GOrgueFrameGeneral::Load(IniFileConfig& cfg, wxString group)
{
	m_ManualNumber = -1;

	NumberOfStops = cfg.ReadInteger(group, wxT("NumberOfStops"), 0, 448);	// the spec says 512, but 64 * 7 = 448
	NumberOfCouplers = cfg.ReadInteger(group, wxT("NumberOfCouplers"), 0, 64);
	NumberOfTremulants = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, 16);
	NumberOfDivisionalCouplers = cfg.ReadInteger(group, wxT("NumberOfDivisionalCouplers"), 0, 8, organfile->GeneralsStoreDivisionalCouplers());

	int i, j, k, m;
	wxString buffer;

	for (i = 0; i < NumberOfStops; i++)
	{
		buffer.Printf(wxT("StopManual%03d"), i + 1);
		m = cfg.ReadInteger(group, buffer, organfile->GetFirstManualIndex(), organfile->GetManualAndPedalCount());
		buffer.Printf(wxT("StopNumber%03d"), i + 1);
		j = cfg.ReadInteger(group, buffer, -organfile->GetManual(m)->GetStopCount(), organfile->GetManual(m)->GetStopCount());
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(stop[m], k, 0, true);
			SET_BIT(stop[m], k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfCouplers; i++)
	{
		buffer.Printf(wxT("CouplerManual%03d"), i + 1);
		m = cfg.ReadInteger(group, buffer, organfile->GetFirstManualIndex(), organfile->GetManualAndPedalCount());
		buffer.Printf(wxT("CouplerNumber%03d"), i + 1);
		j = cfg.ReadInteger(group, buffer, -organfile->GetManual(m)->GetCouplerCount(), organfile->GetManual(m)->GetCouplerCount());
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(coupler[m], k, 0, true);
			SET_BIT(coupler[m], k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfTremulants; i++)
	{
		buffer.Printf(wxT("TremulantNumber%03d"), i + 1);
		j = cfg.ReadInteger(group, buffer, -organfile->GetTremulantCount(), organfile->GetTremulantCount());
		k = abs(j) - 1;
		if (k >= 0)
		{
			SET_BIT(tremulant, k, 0, true);
			SET_BIT(tremulant, k, 1, j >= 0);
		}
	}
	for (i = 0; i < NumberOfDivisionalCouplers; i++)
	{
		buffer.Printf(wxT("DivisionalCouplerNumber%03d"), i + 1);
		j = cfg.ReadInteger( group, buffer, -organfile->GetDivisionalCouplerCount(), organfile->GetDivisionalCouplerCount());
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

	if (g_sound->GetMidi().SetterActive())
	{
		NumberOfStops = NumberOfCouplers = NumberOfTremulants = NumberOfDivisionalCouplers = 0;
		memset(stop, 0, sizeof(stop) + sizeof(coupler) + sizeof(tremulant) + sizeof(divisionalcoupler));
		for (j = organfile->GetFirstManualIndex(); j <= organfile->GetManualAndPedalCount(); j++)
		{
			for (i = 0; i < organfile->GetManual(j)->GetStopCount(); i++)
			{
				if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !organfile->GetManual(j)->GetStop(i)->Displayed)
					continue;
				NumberOfStops++;
				SET_BIT(stop[j], i, 0, true);
				SET_BIT(stop[j], i, 1, organfile->GetManual(j)->GetStop(i)->DefaultToEngaged);
			}
		}
		for (j = organfile->GetFirstManualIndex(); j <= organfile->GetManualAndPedalCount(); j++)
		{
			for (i = 0; i < organfile->GetManual(j)->GetCouplerCount(); i++)
			{
				if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !organfile->GetManual(j)->GetCoupler(i)->Displayed)
					continue;
				NumberOfCouplers++;
				SET_BIT(coupler[j], i, 0, true);
				SET_BIT(coupler[j], i, 1, organfile->GetManual(j)->GetCoupler(i)->DefaultToEngaged);
			}
		}
		for (i = 0; i < organfile->GetTremulantCount(); i++)
		{
			if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !organfile->GetTremulant(i)->Displayed)
				continue;
			NumberOfTremulants++;
			SET_BIT(tremulant, i, 0, true);
			SET_BIT(tremulant, i, 1, organfile->GetTremulant(i)->DefaultToEngaged);
		}
		if (organfile->GeneralsStoreDivisionalCouplers())
		{
			for (i = 0; i < organfile->GetDivisionalCouplerCount(); i++)
			{
				NumberOfDivisionalCouplers++;
				SET_BIT(divisionalcoupler, i, 0, true);
				SET_BIT(divisionalcoupler, i, 1, organfile->GetDivisionalCoupler(i)->DefaultToEngaged);
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
	}
	else
	{
		for (k = 0; ;k++)
		{
			for (j = organfile->GetFirstManualIndex(); j <= organfile->GetManualAndPedalCount(); j++)
			{
				for (i = 0; i < organfile->GetManual(j)->GetStopCount(); i++)
					if (GET_BIT(stop[j], i, 0))
						SET_BIT(stop[j], i, 1, k < 2 ? organfile->GetManual(j)->GetStop(i)->Set(GET_BIT(stop[j], i, 1)) : GET_BIT(stop[j], i, 1));
				for (i = 0; i < organfile->GetManual(j)->GetCouplerCount(); i++)
					if (GET_BIT(coupler[j], i, 0))
						SET_BIT(coupler[j], i, 1, k < 2 ? organfile->GetManual(j)->GetCoupler(i)->Set(GET_BIT(coupler[j], i, 1)) : GET_BIT(coupler[j], i, 1));
			}
			for (i = 0; i < organfile->GetTremulantCount(); i++)
				if (GET_BIT(tremulant, i, 0))
					SET_BIT(tremulant, i, 1, k < 2 ? organfile->GetTremulant(i)->Set(GET_BIT(tremulant, i, 1)) : GET_BIT(tremulant, i, 1));
			for (i = 0; i < organfile->GetDivisionalCouplerCount(); i++)
				if (GET_BIT(divisionalcoupler, i, 0))
					SET_BIT(divisionalcoupler, i, 1, k < 2 ? organfile->GetDivisionalCoupler(i)->Set(GET_BIT(divisionalcoupler, i, 1)) : GET_BIT(divisionalcoupler, i, 1));
			if (k > 1)
				break;
			organfile->MIDIPretend(!k);
		}
	}

	wxByte used = 0;
	for (j = organfile->GetFirstManualIndex(); j <= organfile->GetManualAndPedalCount(); j++)
	{
		for (i = 0; i < 8; i++)
			used |= stop[j][i][1];
		for (i = 0; i < 2; i++)
			used |= coupler[j][i][1];
	}
	used |= tremulant[0][1] | tremulant[1][1] | divisionalcoupler[0][1];

	for (k = 0; k < organfile->GetGeneralCount(); k++)
	{
		GOrgueGeneral* general = organfile->GetGeneral(k);
		int on = ((general == this && used) ? 2 : 0);
		if ((general->DispImageNum & 2) != on)
		{
			general->DispImageNum = (general->DispImageNum & 1) | on;
			wxCommandEvent event(wxEVT_PUSHBUTTON, 0);
			event.SetClientData(general);
			::wxGetApp().frame->AddPendingEvent(event);
		}
	}

	for (j = organfile->GetFirstManualIndex(); j <= organfile->GetManualAndPedalCount(); j++)
	{
		for (k = 0; k < organfile->GetManual(j)->GetDivisionalCount(); k++)
		{
			GOrgueDivisional *divisional = organfile->GetManual(j)->GetDivisional(k);
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

	cfg.SaveHelper( prefix, group, wxT("NumberOfStops"), NumberOfStops);
	cfg.SaveHelper( prefix, group, wxT("NumberOfCouplers"), NumberOfCouplers);
	cfg.SaveHelper( prefix, group, wxT("NumberOfTremulants"), NumberOfTremulants);
	cfg.SaveHelper( prefix, group, wxT("NumberOfDivisionalCouplers"), NumberOfDivisionalCouplers);

	int i, j, k;
	wxString buffer;

	for (j = organfile->GetFirstManualIndex(), k = 1; j <= organfile->GetManualAndPedalCount(); j++)
		for (i = 0; i < organfile->GetManual(j)->GetStopCount(); i++)
			if (GET_BIT(stop[j], i, 0))
			{
				buffer.Printf(wxT("StopManual%03d"), k);
				cfg.SaveHelper( prefix, group, buffer, j, true, true);
				buffer.Printf(wxT("StopNumber%03d"), k++);
				cfg.SaveHelper( prefix, group, buffer, GET_BIT(stop[j], i, 1) ? i + 1 : -1 - i, true);
			}
	for (j = organfile->GetFirstManualIndex(), k = 1; j <= organfile->GetManualAndPedalCount(); j++)
		for (i = 0; i < organfile->GetManual(j)->GetCouplerCount(); i++)
			if (GET_BIT(coupler[j], i, 0))
			{
				buffer.Printf(wxT("CouplerManual%03d"), k);
				cfg.SaveHelper( prefix, group, buffer, j, true, true);
				buffer.Printf(wxT("CouplerNumber%03d"), k++);
				cfg.SaveHelper( prefix, group, buffer, GET_BIT(coupler[j], i, 1) ? i + 1 : -1 - i, true);
			}
	for (i = 0, k = 1; i < organfile->GetTremulantCount(); i++)
		if (GET_BIT(tremulant, i, 0))
		{
			buffer.Printf(wxT("TremulantNumber%03d"), k++);
			cfg.SaveHelper( prefix, group, buffer, GET_BIT(tremulant, i, 1) ? i + 1 : -1 - i, true);
		}
	for (i = 0, k = 1; i < organfile->GetDivisionalCouplerCount(); i++)
		if (GET_BIT(divisionalcoupler, i, 0))
		{
			buffer.Printf(wxT("DivisionalCouplerNumber%03d"), k++);
			cfg.SaveHelper( prefix, group, buffer, GET_BIT(divisionalcoupler, i, 1) ? i + 1 : -1 - i, true);
		}
}

