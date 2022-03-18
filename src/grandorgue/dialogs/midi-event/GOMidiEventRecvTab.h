/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTRECVTAB_H
#define GOMIDIEVENTRECVTAB_H

#include <vector>

#include <wx/panel.h>
#include <wx/timer.h>

#include "midi/GOMidiCallback.h"
#include "midi/GOMidiListener.h"
#include "midi/GOMidiReceiverBase.h"

#include "GOChoice.h"

class wxButton;
class wxChoice;
class wxSpinCtrl;
class wxStaticText;
class wxToggleButton;

class GOConfig;
class GOMidiDeviceConfigList;

class GOMidiEventRecvTab : public wxPanel, protected GOMidiCallback {
private:
  GOMidiDeviceConfigList &m_MidiIn;
  GOMidiMap &m_MidiMap;

  GOMidiReceiverBase *m_original;
  GOMidiReceiverData m_midi;
  GOMidiListener m_listener;
  GOChoice<GOMidiReceiveMessageType> *m_eventtype;
  wxChoice *m_eventno, *m_channel, *m_device;
  wxStaticText *m_DataLabel;
  wxSpinCtrl *m_data;
  wxSpinCtrl *m_LowKey;
  wxSpinCtrl *m_HighKey;
  wxStaticText *m_LowValueLabel;
  wxSpinCtrl *m_LowValue;
  wxStaticText *m_HighValueLabel;
  wxSpinCtrl *m_HighValue;
  wxSpinCtrl *m_Debounce;
  wxToggleButton *m_ListenSimple;
  wxStaticText *m_ListenInstructions;
  wxToggleButton *m_ListenAdvanced;
  unsigned m_ListenState;
  wxButton *m_new, *m_delete;
  wxTimer m_Timer;
  int m_current;
  std::vector<GOMidiEvent> m_OnList;
  std::vector<GOMidiEvent> m_OffList;

  bool SimilarEvent(const GOMidiEvent &e1, const GOMidiEvent &e2);

  void StoreEvent();
  void LoadEvent();
  void StartListen(bool type);
  void StopListen();
  void DetectEvent();

  void OnTimer(wxTimerEvent &event);
  void OnListenSimpleClick(wxCommandEvent &event);
  void OnListenAdvancedClick(wxCommandEvent &event);
  void OnNewClick(wxCommandEvent &event);
  void OnDeleteClick(wxCommandEvent &event);
  void OnEventChange(wxCommandEvent &event);
  void OnTypeChange(wxCommandEvent &event);
  void OnMidiEvent(const GOMidiEvent &event);

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
  GOMidiEventRecvTab(
    wxWindow *parent, GOMidiReceiverBase *event, GOConfig &config);
  ~GOMidiEventRecvTab();
  void RegisterMIDIListener(GOMidi *midi);

  virtual bool TransferDataFromWindow() override;
  GOMidiReceiveEvent GetCurrentEvent();

  DECLARE_EVENT_TABLE()
};

#endif /* MIDIEVENTDIALOG_H_ */
