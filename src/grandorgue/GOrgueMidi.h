/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEMIDI_H
#define GORGUEMIDI_H

#include "GOrgueMidiRtFactory.h"
#include "ptrvector.h"
#include <wx/event.h>

class GOrgueMidiEvent;
class GOrgueMidiInPort;
class GOrgueMidiListener;
class GOrgueMidiMap;
class GOrgueMidiOutPort;
class GOrgueSettings;
class GrandOrgueFile;
class wxMidiEvent;

class GOrgueMidi : public wxEvtHandler
{
private:
	GOrgueSettings& m_Settings;
	ptr_vector<GOrgueMidiInPort> m_midi_in_devices;
	ptr_vector<GOrgueMidiOutPort> m_midi_out_devices;
	int m_transpose;
	std::vector<GOrgueMidiListener*> m_Listeners;
	GOrgueMidiRtFactory m_MidiFactory;
	void OnMidiEvent(wxMidiEvent& event);

public:

	GOrgueMidi(GOrgueSettings& settings);
	~GOrgueMidi();

	void Open();
	void UpdateDevices();

	void Recv(const GOrgueMidiEvent& e);
	void Send(const GOrgueMidiEvent& e);

	std::vector<wxString> GetInDevices();
	std::vector<wxString> GetOutDevices();
	bool HasActiveDevice();
	int GetTranspose();
	void SetTranspose(int transpose);
	void Register(GOrgueMidiListener* listener);
	void Unregister(GOrgueMidiListener* listener);

	GOrgueMidiMap& GetMidiMap();

	DECLARE_EVENT_TABLE()
};

#endif /* GORGUEMIDI_H */
