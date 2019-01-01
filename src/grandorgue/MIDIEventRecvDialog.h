/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MIDIEVENTRECVDIALOG_H_
#define MIDIEVENTRECVDIALOG_H_

#include "GOrgueChoice.h"
#include "GOrgueMidiCallback.h"
#include "GOrgueMidiListener.h"
#include "GOrgueMidiReceiverBase.h"
#include <wx/panel.h>
#include <wx/timer.h>
#include <vector>

class GOrgueSettings;
class wxButton;
class wxChoice;
class wxSpinCtrl;
class wxStaticText;
class wxToggleButton;

class MIDIEventRecvDialog : public wxPanel, protected GOrgueMidiCallback
{
private:
	GOrgueSettings& m_Settings;
	GOrgueMidiReceiverBase* m_original;
	GOrgueMidiReceiverData m_midi;
	GOrgueMidiListener m_listener;
	GOrgueChoice<midi_match_message_type>* m_eventtype;
	wxChoice *m_eventno, *m_channel, *m_device;
	wxStaticText* m_DataLabel;
	wxSpinCtrl *m_data;
	wxSpinCtrl *m_LowKey;
	wxSpinCtrl *m_HighKey;
	wxStaticText *m_LowValueLabel;
	wxSpinCtrl *m_LowValue;
	wxStaticText *m_HighValueLabel;
	wxSpinCtrl *m_HighValue;
	wxSpinCtrl *m_Debounce;
	wxToggleButton* m_ListenSimple;
	wxStaticText* m_ListenInstructions;
	wxToggleButton* m_ListenAdvanced;
	unsigned m_ListenState;
	wxButton* m_new, *m_delete;
	wxTimer m_Timer;
	int m_current;
	std::vector<GOrgueMidiEvent> m_OnList;
	std::vector<GOrgueMidiEvent> m_OffList;

	bool SimilarEvent(const GOrgueMidiEvent& e1, const GOrgueMidiEvent& e2);

	void StoreEvent();
	void LoadEvent();
	void StartListen(bool type);
	void StopListen();
	void DetectEvent();

	void OnTimer(wxTimerEvent& event);
	void OnListenSimpleClick(wxCommandEvent& event);
	void OnListenAdvancedClick(wxCommandEvent& event);
	void OnNewClick(wxCommandEvent& event);
	void OnDeleteClick(wxCommandEvent& event);
	void OnEventChange(wxCommandEvent& event);
	void OnTypeChange(wxCommandEvent& event);
	void OnMidiEvent(const GOrgueMidiEvent& event);

protected:
	enum {
		ID_EVENT_NO = 200,
		ID_EVENT_NEW,
		ID_EVENT_DELETE,
		ID_DEVICE,
		ID_EVENT,
		ID_CHANNEL,
		ID_DATA,
		ID_LISTEN_SIMPLE,
		ID_LISTEN_ADVANCED,
		ID_TIMER,
		ID_LOW_KEY,
		ID_HIGH_KEY,
		ID_LOW_VALUE,
		ID_HIGH_VALUE,
		ID_DEBOUNCE
	};

public:
	MIDIEventRecvDialog (wxWindow* parent, GOrgueMidiReceiverBase* event, GOrgueSettings& settings);
	~MIDIEventRecvDialog();
	void RegisterMIDIListener(GOrgueMidi* midi);

	void DoApply();
	MIDI_MATCH_EVENT GetCurrentEvent();

	DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
