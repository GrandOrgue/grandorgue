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

#include "GOrgueSettings.h"

#include <wx/wx.h>
#include <wx/confbase.h>

GOrgueSettings::GOrgueSettings() :
	m_Config(*wxConfigBase::Get()),
	m_Stereo(false),
	m_Concurrency(0),
	m_ReleaseConcurrency(1)
{
}

void GOrgueSettings::Load()
{
	m_Stereo = m_Config.Read(wxT("StereoEnabled"), 1);
	m_Concurrency = m_Config.Read(wxT("Concurrency"), 0L);
	m_ReleaseConcurrency = m_Config.Read(wxT("ReleaseConcurrency"), 1L);
	if (m_ReleaseConcurrency < 1)
		m_ReleaseConcurrency = 1;
}

bool GOrgueSettings::GetLoadInStereo()
{
	return m_Stereo;
}

void GOrgueSettings::SetLoadInStereo(bool stereo)
{
	m_Stereo = stereo;
	m_Config.Write(wxT("StereoEnabled"), m_Stereo);
}

unsigned GOrgueSettings::GetConcurrency()
{
	return m_Concurrency;
}

void GOrgueSettings::SetConcurrency(unsigned concurrency)
{
	m_Concurrency = concurrency;
	m_Config.Write(wxT("Concurrency"), (long)m_Concurrency);
}

unsigned GOrgueSettings::GetReleaseConcurrency()
{
	return m_ReleaseConcurrency;
}

void GOrgueSettings::SetReleaseConcurrency(unsigned concurrency)
{
	if (concurrency < 1)
		concurrency = 1;
	m_ReleaseConcurrency = concurrency;
	m_Config.Write(wxT("ReleaseConcurrency"), (long)m_ReleaseConcurrency);
}
