/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#include "GOrgueSetterButton.h"

#include "GOrgueSetterButtonCallback.h"
#include <wx/intl.h>

GOrgueSetterButton::GOrgueSetterButton(GrandOrgueFile* organfile, GOrgueSetterButtonCallback* setter, bool Pushbutton) :
	GOrgueButton(organfile, MIDI_RECV_SETTER, Pushbutton),
	m_setter(setter)
{
}

void GOrgueSetterButton::Push()
{
	if (m_Pushbutton)
		m_setter->SetterButtonChanged(this);
	else
		GOrgueButton::Push();
}

void GOrgueSetterButton::Set(bool on)
{
	if (IsEngaged() == on)
		return;
	m_setter->SetterButtonChanged(this);
	Display(on);
}

wxString GOrgueSetterButton::GetMidiType()
{
	return _("Button");
}
