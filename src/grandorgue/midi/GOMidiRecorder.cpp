/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiRecorder.h"

#include <wx/filename.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/stopwatch.h>

#include "config/GOConfig.h"
#include "control/GOCallbackButtonControl.h"
#include "midi/GOMidiEvent.h"
#include "midi/GOMidiFile.h"

#include "GOEvent.h"
#include "GOMidi.h"
#include "GOOrganController.h"
#include "GOPath.h"

enum {
  ID_MIDI_RECORDER_RECORD = 0,
  ID_MIDI_RECORDER_STOP,
  ID_MIDI_RECORDER_RECORD_RENAME,
};

const struct GOElementCreator::ButtonDefinitionEntry
  GOMidiRecorder::m_element_types[]
  = {
    {wxT("MidiRecorderRecord"), ID_MIDI_RECORDER_RECORD, false, true, false},
    {wxT("MidiRecorderStop"), ID_MIDI_RECORDER_STOP, false, true, false},
    {wxT("MidiRecorderRecordRename"),
     ID_MIDI_RECORDER_RECORD_RENAME,
     false,
     true,
     false},
    {wxT(""), -1, false, false, false},
};

const struct GOElementCreator::ButtonDefinitionEntry *GOMidiRecorder::
  GetButtonDefinitionList() {
  return m_element_types;
}

GOMidiRecorder::GOMidiRecorder(GOOrganController *organController)
  : m_OrganController(organController),
    m_Map(organController->GetSettings().GetMidiMap()),
    m_RecordingTime(organController),
    m_RecordSeconds(0),
    m_NextChannel(0),
    m_NextNRPN(0),
    m_Mappings(),
    m_Preconfig(),
    m_OutputDevice(0),
    m_file(),
    m_Filename(),
    m_DoRename(false),
    m_BufferPos(0),
    m_FileLength(0),
    m_Last(0) {
  CreateButtons(*m_OrganController);
  Clear();
  UpdateDisplay();
}

GOMidiRecorder::~GOMidiRecorder() { StopRecording(); }

void GOMidiRecorder::Load(GOConfigReader &cfg) {
  m_buttons[ID_MIDI_RECORDER_RECORD]->Init(
    cfg, wxT("MidiRecorderRecord"), _("REC"));
  m_buttons[ID_MIDI_RECORDER_STOP]->Init(
    cfg, wxT("MidiRecorderStop"), _("STOP"));
  m_buttons[ID_MIDI_RECORDER_RECORD_RENAME]->Init(
    cfg, wxT("MidiRecorderRecordRename"), _("REC File"));
  m_RecordingTime.Init(cfg, wxT("MidiRecordTime"), _("MIDI recording time"));
}

void GOMidiRecorder::ButtonStateChanged(int id, bool newState) {
  switch (id) {
  case ID_MIDI_RECORDER_STOP:
    StopRecording();
    break;

  case ID_MIDI_RECORDER_RECORD:
    StartRecording(false);
    break;

  case ID_MIDI_RECORDER_RECORD_RENAME:
    StartRecording(true);
    break;
  }
}

void GOMidiRecorder::SetOutputDevice(const wxString &device_id) {
  m_OutputDevice = m_Map.GetDeviceIdByLogicalName(device_id);
}

void GOMidiRecorder::SendEvent(GOMidiEvent &e) {
  e.SetDevice(m_OutputDevice);
  if (m_OutputDevice)
    m_OrganController->SendMidiMessage(e);
  if (IsRecording())
    WriteEvent(e);
}

void GOMidiRecorder::Clear() {
  m_Mappings.clear();
  m_Preconfig.clear();
  m_NextChannel = 1;
  m_NextNRPN = 0;

  GOMidiEvent e;
  e.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_CLEAR);
  e.SetTime(wxGetLocalTimeMillis());
  SendEvent(e);
}

void GOMidiRecorder::PreconfigureMapping(const wxString &element, bool isNRPN) {
  PreconfigureMapping(element, isNRPN, element);
}

void GOMidiRecorder::PreconfigureMapping(
  const wxString &element, bool isNRPN, const wxString &reference) {
  unsigned id = m_Map.GetElementByString(element);
  unsigned ref = m_Map.GetElementByString(reference);
  for (unsigned i = 0; i < m_Preconfig.size(); i++)
    if (m_Preconfig[i].elementID == ref) {
      GOMidiEvent e1;
      e1.SetTime(wxGetLocalTimeMillis());
      e1.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SETUP);
      e1.SetKey(id);
      e1.SetChannel(m_Preconfig[i].channel);
      e1.SetValue(m_Preconfig[i].key);
      SendEvent(e1);

      midi_map m = m_Preconfig[i];
      m.elementID = ref;
      if (ref >= m_Mappings.size())
        m_Mappings.resize(ref + 1);
      m_Mappings[ref] = m;
      return;
    }

  midi_map m;
  if (isNRPN) {
    if (m_NextNRPN >= 1 << 18)
      return;
    m.elementID = ref;
    m.channel = 1 + m_NextNRPN / (1 << 14);
    m.key = m_NextNRPN % (1 << 14);
    m_NextNRPN++;
  } else {
    if (m_NextChannel > 16)
      return;
    m.elementID = ref;
    m.channel = m_NextChannel;
    m.key = 0;
    m_NextChannel++;
  }
  m_Preconfig.push_back(m);

  GOMidiEvent e1;
  e1.SetTime(wxGetLocalTimeMillis());
  e1.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SETUP);
  e1.SetKey(id);
  e1.SetChannel(m.channel);
  e1.SetValue(m.key);
  SendEvent(e1);

  if (ref >= m_Mappings.size())
    m_Mappings.resize(ref + 1);
  m_Mappings[ref] = m;
}

void GOMidiRecorder::SetSamplesetId(unsigned id1, unsigned id2) {
  GOMidiEvent e1;
  e1.SetTime(wxGetLocalTimeMillis());
  e1.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SAMPLESET);
  e1.SetKey(id1);
  e1.SetValue(id2);
  SendEvent(e1);

  m_Mappings.clear();
}

bool GOMidiRecorder::SetupMapping(unsigned element, bool isNRPN) {
  if (element >= m_Mappings.size())
    m_Mappings.resize(element + 1);

  if (element != m_Mappings[element].elementID) {
    if (isNRPN) {
      if (m_NextNRPN >= 1 << 18)
        return false;
      m_Mappings[element].elementID = element;
      m_Mappings[element].channel = 1 + m_NextNRPN / (1 << 14);
      m_Mappings[element].key = m_NextNRPN % (1 << 14);
      m_NextNRPN++;
    } else {
      if (m_NextChannel > 16)
        return false;
      m_Mappings[element].elementID = element;
      m_Mappings[element].channel = m_NextChannel;
      m_Mappings[element].key = 0;
      m_NextChannel++;
    }
    GOMidiEvent e1;
    e1.SetTime(wxGetLocalTimeMillis());
    e1.SetMidiType(GOMidiEvent::MIDI_SYSEX_GO_SETUP);
    e1.SetKey(m_Mappings[element].elementID);
    e1.SetChannel(m_Mappings[element].channel);
    e1.SetValue(m_Mappings[element].key);
    SendEvent(e1);
  }
  return true;
}

void GOMidiRecorder::SendMidiRecorderMessage(GOMidiEvent &e) {
  if (!m_OutputDevice && !IsRecording())
    return;
  if (!SetupMapping(e.GetDevice(), e.GetMidiType() == GOMidiEvent::MIDI_NRPN))
    return;

  e.SetTime(wxGetLocalTimeMillis());
  e.SetChannel(m_Mappings[e.GetDevice()].channel);
  if (e.GetMidiType() == GOMidiEvent::MIDI_NRPN)
    e.SetKey(m_Mappings[e.GetDevice()].key);

  SendEvent(e);
}

void GOMidiRecorder::Flush() {
  if (!m_BufferPos)
    return;
  m_file.Write(m_Buffer, m_BufferPos);
  m_FileLength += m_BufferPos;
  m_BufferPos = 0;
}

void GOMidiRecorder::Ensure(unsigned length) {
  if (m_BufferPos + length < sizeof(m_Buffer))
    return;
  Flush();
}

void GOMidiRecorder::Write(const void *data, unsigned len) {
  if (len >= sizeof(m_Buffer)) {
    Flush();
    m_file.Write(data, len);
    m_FileLength += len;
    return;
  }
  Ensure(len);
  memcpy(m_Buffer + m_BufferPos, data, len);
  m_BufferPos += len;
}

void GOMidiRecorder::EncodeLength(unsigned len) {
  char buf[16];
  unsigned l = sizeof(buf) - 1;
  buf[l] = len & 0x7F;
  len = len >> 7;
  while (len) {
    l--;
    buf[l] = 0x80 | (len & 0x7F);
    len = len >> 7;
  }
  Write(buf + l, sizeof(buf) - l);
}

void GOMidiRecorder::WriteEvent(GOMidiEvent &e) {
  if (!IsRecording())
    return;
  std::vector<std::vector<unsigned char>> msg;
  e.ToMidi(msg, m_Map);
  for (unsigned i = 0; i < msg.size(); i++) {
    EncodeLength((e.GetTime() - m_Last).GetValue());
    if (msg[i][0] == 0xF0) {
      Write(&msg[i][0], 1);
      EncodeLength(msg[i].size() - 1);
      Write(&msg[i][1], msg[i].size() - 1);
    } else
      Write(&msg[i][0], msg[i].size());
    m_Last = e.GetTime();
  }
}

bool GOMidiRecorder::IsRecording() { return m_file.IsOpened(); }

void GOMidiRecorder::UpdateDisplay() {
  if (!IsRecording())
    m_RecordingTime.SetContent(_("-:--:--"));
  else
    m_RecordingTime.SetContent(wxString::Format(
      _("%d:%02d:%02d"),
      m_RecordSeconds / 3600,
      (m_RecordSeconds / 60) % 60,
      m_RecordSeconds % 60));
}

void GOMidiRecorder::StopRecording() {
  m_buttons[ID_MIDI_RECORDER_RECORD]->Display(false);
  m_buttons[ID_MIDI_RECORDER_RECORD_RENAME]->Display(false);
  m_OrganController->DeleteTimer(this);
  if (!IsRecording())
    return;
  const unsigned char end[4] = {0x01, 0xFF, 0x2F, 0x00};
  Write(end, sizeof(end));
  Flush();
  m_FileLength -= sizeof(MIDIHeaderChunk) + sizeof(MIDIFileHeader);
  MIDIFileHeader h = {{'M', 'T', 'r', 'k'}, m_FileLength};
  m_file.Seek(sizeof(MIDIHeaderChunk));
  m_file.Write(&h, sizeof(h));
  m_file.Flush();
  m_file.Close();
  if (!m_DoRename) {
    wxFileName name = m_Filename;
    GOSyncDirectory(name.GetPath());
  } else
    GOAskRenameFile(
      m_Filename,
      m_OrganController->GetSettings().MidiRecorderPath(),
      _("MIDI files (*.mid)|*.mid"));
  UpdateDisplay();
}

void GOMidiRecorder::StartRecording(bool rename) {
  MIDIHeaderChunk h = {{{'M', 'T', 'h', 'd'}, 6}, 0, 1, 0xE728};
  MIDIFileHeader t = {{'M', 'T', 'r', 'k'}, 0};

  StopRecording();
  if (!m_OrganController)
    return;

  m_Filename = m_OrganController->GetSettings().MidiRecorderPath()
    + wxFileName::GetPathSeparator()
    + wxDateTime::UNow().Format(_("%Y-%m-%d-%H-%M-%S.%l.mid"));
  m_DoRename = rename;

  m_file.Create(m_Filename, true);
  if (!m_file.IsOpened()) {
    wxLogError(_("Unable to open file %s for writing"), m_Filename.c_str());
    return;
  }

  m_FileLength = 0;
  m_BufferPos = 0;
  Write(&h, sizeof(h));
  Write(&t, sizeof(t));

  wxString s = m_OrganController->GetChurchName();
  wxCharBuffer b = s.ToAscii();
  unsigned len = s.length();
  unsigned char th[] = {0x00, 0xFF, 0x04};
  Write(&th, sizeof(th));
  EncodeLength(len);
  Write(b.data(), len);

  m_Last = wxGetLocalTimeMillis();
  if (m_DoRename)
    m_buttons[ID_MIDI_RECORDER_RECORD_RENAME]->Display(true);
  else
    m_buttons[ID_MIDI_RECORDER_RECORD]->Display(true);
  m_OrganController->PrepareRecording();

  m_RecordSeconds = 0;
  UpdateDisplay();
  m_OrganController->SetRelativeTimer(1000, this, 1000);
}

GOEnclosure *GOMidiRecorder::GetEnclosure(const wxString &name, bool is_panel) {
  return NULL;
}

GOLabelControl *GOMidiRecorder::GetLabelControl(
  const wxString &name, bool is_panel) {
  if (is_panel)
    return NULL;

  if (name == wxT("MidiRecorderLabel"))
    return &m_RecordingTime;
  return NULL;
}

void GOMidiRecorder::HandleTimer() {
  m_RecordSeconds++;
  UpdateDisplay();
}
