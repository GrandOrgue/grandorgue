/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueWindchest.h"

#include "GOrgueConfigReader.h"
#include "GOrgueEnclosure.h"
#include "GOrguePipeWindchestCallback.h"
#include "GOrgueTremulant.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOrgueWindchest::GOrgueWindchest(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_Name(),
	m_Volume(1),
	m_enclosure(0),
	m_tremulant(0),
	m_ranks(0),
	m_pipes(0),
	m_PipeConfig(&organfile->GetPipeConfig(), organfile, NULL)
{
	m_organfile->RegisterPlaybackStateHandler(this);
}

void GOrgueWindchest::Init(GOrgueConfigReader& cfg, wxString group, wxString name)
{
	m_enclosure.resize(0);
	m_tremulant.resize(0);
	m_Name = name;
	m_PipeConfig.Init(cfg, group, wxEmptyString);
	m_PipeConfig.SetName(GetName());
}


void GOrgueWindchest::Load(GOrgueConfigReader& cfg, wxString group, unsigned index)
{
	unsigned NumberOfEnclosures = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfEnclosures"), 0, m_organfile->GetEnclosureCount());
	unsigned NumberOfTremulants = cfg.ReadInteger(ODFSetting, group, wxT("NumberOfTremulants"), 0, m_organfile->GetTremulantCount());

	m_enclosure.resize(0);
	for (unsigned i = 0; i < NumberOfEnclosures; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Enclosure%03d"), i + 1);
		m_enclosure.push_back(m_organfile->GetEnclosureElement(cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetEnclosureCount()) - 1));
	}

	m_tremulant.resize(0);
	for (unsigned i = 0; i < NumberOfTremulants; i++)
	{
		wxString buffer;
		buffer.Printf(wxT("Tremulant%03d"), i + 1);
		m_tremulant.push_back(cfg.ReadInteger(ODFSetting, group, buffer, 1, m_organfile->GetTremulantCount()) - 1);
	}

	m_Name = cfg.ReadStringNotEmpty(ODFSetting, group, wxT("Name"), false, wxString::Format(_("Windchest %d"), index + 1));
	m_PipeConfig.Init(cfg, group, wxEmptyString);
	m_PipeConfig.SetName(GetName());
}

void GOrgueWindchest::UpdateVolume()
{
	float f = 1.0f;
	for (unsigned i = 0; i < m_enclosure.size(); i++)
		f *= m_enclosure[i]->GetAttenuation();
	m_Volume = f;
}

float GOrgueWindchest::GetVolume()
{
	return m_Volume;
}

unsigned GOrgueWindchest::GetTremulantCount()
{
	return m_tremulant.size();
}

unsigned GOrgueWindchest::GetTremulantId(unsigned no)
{
	return m_tremulant[no];
}

unsigned GOrgueWindchest::GetRankCount()
{
	return m_ranks.size();
}

GOrgueRank* GOrgueWindchest::GetRank(unsigned index)
{
	return m_ranks[index];
}

void GOrgueWindchest::AddRank(GOrgueRank* rank)
{
	m_ranks.push_back(rank);
}

void GOrgueWindchest::AddPipe(GOrguePipeWindchestCallback * pipe)
{
	m_pipes.push_back(pipe);
}

void GOrgueWindchest::AddEnclosure(GOrgueEnclosure* enclosure)
{
	m_enclosure.push_back(enclosure);
}

const wxString& GOrgueWindchest::GetName()
{
	return m_Name;
}

GOrguePipeConfigNode& GOrgueWindchest::GetPipeConfig()
{
	return m_PipeConfig;
}

void GOrgueWindchest::UpdateTremulant(GOrgueTremulant* tremulant)
{
	for (unsigned i = 0; i < m_tremulant.size(); i++)
		if (tremulant == m_organfile->GetTremulant(m_tremulant[i]))
		{
			GOrgueTremulant* t = m_organfile->GetTremulant(m_tremulant[i]);
			if (t->GetTremulantType() != GOWavTrem)
				continue;
			bool on = t->IsActive();
			for(unsigned j = 0; j < m_pipes.size(); j++)
				m_pipes[j]->SetTremulant(on);
			return;
		}
}

void GOrgueWindchest::AbortPlayback()
{
}

void GOrgueWindchest::StartPlayback()
{
}

void GOrgueWindchest::PreparePlayback()
{
	UpdateVolume();
}

void GOrgueWindchest::PrepareRecording()
{
}
