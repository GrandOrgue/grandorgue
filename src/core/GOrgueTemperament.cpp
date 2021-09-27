/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueTemperament.h"

#include <wx/intl.h>

GOrgueTemperament::GOrgueTemperament(wxString name, wxString group) :
	m_Group(group),
	m_Name(name),
	m_Title(wxGetTranslation(name))
{
}

GOrgueTemperament::GOrgueTemperament(wxString name, wxString title, wxString group) :
	m_Group(group),
	m_Name(name),
	m_Title(title)
{
}

GOrgueTemperament::~GOrgueTemperament()
{
}

float GOrgueTemperament::GetOffset(bool ignorepitch, unsigned midi_number, unsigned wav_midi_number, float wav_pitch_fract, float harmonic_number, float pitch_correction, float default_tuning) const
{
	return 0;
}

wxString GOrgueTemperament::GetName() const
{
	return m_Name;
}

wxString GOrgueTemperament::GetTitle() const
{
	return m_Title;
}

wxString GOrgueTemperament::GetGroup() const
{
	return m_Group;
}
