/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOSetterButton.h"

#include "GOSetterButtonCallback.h"
#include <wx/intl.h>

GOSetterButton::GOSetterButton(GODefinitionFile* organfile, GOSetterButtonCallback* setter, bool Pushbutton) :
	GOButton(organfile, MIDI_RECV_SETTER, Pushbutton),
	m_setter(setter)
{
}

void GOSetterButton::Push()
{
	if (m_Pushbutton)
		m_setter->SetterButtonChanged(this);
	else
		GOButton::Push();
}

void GOSetterButton::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	m_setter->SetterButtonChanged(this);
	Display(on);
}

wxString GOSetterButton::GetMidiType()
{
	return _("Button");
}
