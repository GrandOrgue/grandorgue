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

GOrgueDivisional::GOrgueDivisional() :
	GOrguePushbutton(),
	m_DivisionalNumber(0),
	m_Stops(),
	m_Couplers(),
	m_Tremulants()
{
}

void GOrgueDivisional::Load(IniFileConfig& cfg, wxString group, int manualNumber, int divisionalNumber, GOrgueDisplayMetrics* displayMetrics)
{

	int i;
	wxString buffer;

	m_DivisionalNumber = divisionalNumber;
	m_ManualNumber = manualNumber;
	m_Stops.clear();
	m_Couplers.clear();
	m_Tremulants.clear();

	GOrgueManual* associatedManual = organfile->GetManual(m_ManualNumber);

	int NumberOfStops = cfg.ReadInteger(group, wxT("NumberOfStops"), 0, associatedManual->GetStopCount());
	int NumberOfCouplers = cfg.ReadInteger(group, wxT("NumberOfCouplers"), 0, associatedManual->GetCouplerCount(), organfile->DivisionalsStoreIntermanualCouplers() || organfile->DivisionalsStoreIntramanualCouplers());
	int NumberOfTremulants = cfg.ReadInteger(group, wxT("NumberOfTremulants"), 0, organfile->GetTremulantCount(), organfile->DivisionalsStoreTremulants());

	m_Stops.resize(NumberOfStops);
	for (i = 0; i < NumberOfStops; i++)
	{
		buffer.Printf(wxT("Stop%03d"), i + 1);
		m_Stops[i] = cfg.ReadInteger( group, buffer, -associatedManual->GetStopCount(), associatedManual->GetStopCount());
	}

	if (organfile->DivisionalsStoreIntermanualCouplers() || organfile->DivisionalsStoreIntramanualCouplers())
	{
		m_Couplers.resize(NumberOfCouplers);
		for (i = 0; i < NumberOfCouplers; i++)
		{
			buffer.Printf(wxT("Coupler%03d"), i + 1);
			m_Couplers[i] = cfg.ReadInteger( group, buffer, -associatedManual->GetCouplerCount(), associatedManual->GetCouplerCount());
		}
	}

	if (organfile->DivisionalsStoreTremulants())
	{
		m_Tremulants.resize(NumberOfTremulants);
		for (i = 0; i < NumberOfTremulants; i++)
		{
			buffer.Printf(wxT("Tremulant%03d"), i + 1);
			m_Tremulants[i] = cfg.ReadInteger( group, buffer, -associatedManual->GetTremulantCount(), associatedManual->GetTremulantCount());
		}
	}

	GOrguePushbutton::Load(cfg, group, displayMetrics);

}

void GOrgueDivisional::Save(IniFileConfig& cfg, bool prefix, wxString group)
{

	unsigned int i;
	wxString buffer;

	GOrguePushbutton::Save(cfg, prefix, group);

	cfg.SaveHelper(prefix, group, wxT("NumberOfStops"), m_Stops.size());
	cfg.SaveHelper(prefix, group, wxT("NumberOfCouplers"), m_Couplers.size());
	cfg.SaveHelper(prefix, group, wxT("NumberOfTremulants"), m_Tremulants.size());

	for (i = 0; i < m_Stops.size(); i++)
	{
		buffer.Printf(wxT("Stop%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_Stops[i], true);
	}

	for (i = 0; i < m_Couplers.size(); i++)
	{
		buffer.Printf(wxT("Coupler%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_Couplers[i], true);
	}

	for (i = 0; i < m_Tremulants.size(); i++)
	{
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		cfg.SaveHelper(prefix, group, buffer, m_Tremulants[i], true);
	}

}

void GOrgueDivisional::Push(int depth)
{

	bool used = false;
	GOrgueManual* associatedManual = organfile->GetManual(m_ManualNumber);

	if (g_sound->GetMidi().SetterActive())
	{

		m_Stops.clear();
		m_Couplers.clear();
		m_Tremulants.clear();
		for (int i = 0; i < associatedManual->GetStopCount(); i++)
		{
			if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetStop(i)->Displayed)
				continue;
			m_Stops.push_back(associatedManual->GetStop(i)->DefaultToEngaged ? i + 1 : -i - 1);
			used |= associatedManual->GetStop(i)->DefaultToEngaged;
		}
		for (int i = 0; i < associatedManual->GetCouplerCount(); i++)
		{
			if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetCoupler(i)->Displayed)
				continue;
			if ((organfile->DivisionalsStoreIntramanualCouplers() && m_ManualNumber == associatedManual->GetCoupler(i)->DestinationManual) || (organfile->DivisionalsStoreIntermanualCouplers() && m_ManualNumber != associatedManual->GetCoupler(i)->DestinationManual))
			{
				m_Couplers.push_back(associatedManual->GetCoupler(i)->DefaultToEngaged ? i + 1 : -i - 1);
				used |= associatedManual->GetCoupler(i)->DefaultToEngaged;
			}
		}
		if (organfile->DivisionalsStoreTremulants())
		{
			for (int i = 0; i < associatedManual->GetTremulantCount(); i++)
			{
				if (!organfile->CombinationsStoreNonDisplayedDrawstops() && !associatedManual->GetTremulant(i)->Displayed)
					continue;
				m_Tremulants.push_back(associatedManual->GetTremulant(i)->DefaultToEngaged ? i + 1 : -i - 1);
				used |= associatedManual->GetTremulant(i)->DefaultToEngaged;
			}
		}
		::wxGetApp().m_docManager->GetCurrentDocument()->Modify(true);
		depth = 1;
	}
	else
	{
		for (unsigned i = 0; i < m_Stops.size(); i++)
		{
			if (!m_Stops[i])
				continue;
			unsigned k = abs(m_Stops[i]) - 1;
			associatedManual->GetStop(k)->Set(m_Stops[i] > 0);
			used |= m_Stops[i] > 0;
		}

		for (unsigned i = 0; i < m_Couplers.size(); i++)
		{
			if (!m_Couplers[i])
				continue;
			unsigned k = abs(m_Couplers[i]) - 1;
			associatedManual->GetCoupler(k)->Set(m_Couplers[i] > 0);
			used |= m_Couplers[i] > 0;
		}

		for (unsigned i = 0; i < m_Tremulants.size(); i++)
		{
			if (!m_Tremulants[i])
				continue;
			unsigned k = abs(m_Tremulants[i]) - 1;
			associatedManual->GetTremulant(k)->Set(m_Tremulants[i] > 0);
			used |= m_Tremulants[i] > 0;
		}
	}

	for (int k = 0; k < associatedManual->GetDivisionalCount(); k++)
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

	for (int k = 0; k < organfile->GetDivisionalCouplerCount(); k++)
	{

		GOrgueDivisionalCoupler* coupler = organfile->GetDivisionalCoupler(k);
		if (!coupler->DefaultToEngaged)
			continue;

		for (int i = 0; i < coupler->NumberOfManuals; i++)
		{

			if (coupler->manual[i] != m_ManualNumber)
				continue;

			for (int j = i + 1; j < coupler->NumberOfManuals; j++)
				organfile->GetManual(coupler->manual[j])->GetDivisional(m_DivisionalNumber)->Push(depth + 1);

			if (coupler->BiDirectionalCoupling)
			{

				for (int j = 0; j < coupler->NumberOfManuals; j++)
				{
					if (coupler->manual[j] == m_ManualNumber)
						break;
					organfile->GetManual(coupler->manual[j])->GetDivisional(m_DivisionalNumber)->Push(depth + 1);
				}

			}

			break;

		}

	}

}

