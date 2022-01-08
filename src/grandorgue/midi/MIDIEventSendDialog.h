/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef MIDIEVENTSENDDIALOG_H_
#define MIDIEVENTSENDDIALOG_H_

#include <wx/panel.h>

#include "GOChoice.h"
#include "GOMidiSender.h"

class GOConfig;
class wxButton;
class wxChoice;
class wxSpinCtrl;
class wxStaticText;
class MIDIEventRecvDialog;

class MIDIEventSendDialog : public wxPanel {
 private:
  GOConfig &m_config;
  GOMidiSender *m_original;
  MIDIEventRecvDialog *m_recv;
  GOMidiSenderData m_midi;
  GOChoice<midi_send_message_type> *m_eventtype;
  wxChoice *m_eventno, *m_channel, *m_device;
  wxStaticText *m_KeyLabel;
  wxSpinCtrl *m_key;
  wxStaticText *m_LowValueLabel;
  wxSpinCtrl *m_LowValue;
  wxStaticText *m_HighValueLabel;
  wxSpinCtrl *m_HighValue;
  wxStaticText *m_StartLabel;
  wxSpinCtrl *m_StartValue;
  wxStaticText *m_LengthLabel;
  wxSpinCtrl *m_LengthValue;
  wxButton *m_new, *m_delete, *m_copy;
  int m_current;

  void StoreEvent();
  void LoadEvent();
  MIDI_SEND_EVENT CopyEvent();

  void OnNewClick(wxCommandEvent &event);
  void OnDeleteClick(wxCommandEvent &event);
  void OnEventChange(wxCommandEvent &event);
  void OnTypeChange(wxCommandEvent &event);
  void OnCopyClick(wxCommandEvent &event);

 protected:
  enum {
    ID_EVENT_NO = 200,
    ID_EVENT_NEW,
    ID_EVENT_DELETE,
    ID_DEVICE,
    ID_EVENT,
    ID_CHANNEL,
    ID_KEY,
    ID_LOW_VALUE,
    ID_HIGH_VALUE,
    ID_START,
    ID_LENGTH,
    ID_COPY
  };

 public:
  MIDIEventSendDialog(
    wxWindow *parent,
    GOMidiSender *event,
    MIDIEventRecvDialog *recv,
    GOConfig &settings);
  ~MIDIEventSendDialog();

  void DoApply();

  DECLARE_EVENT_TABLE()
};

#endif
