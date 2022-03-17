/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOAudioRecorder.h"

#include <wx/filename.h>
#include <wx/intl.h>

#include "GODefinitionFile.h"
#include "GOEvent.h"
#include "GOPath.h"
#include "GOSetterButton.h"
#include "config/GOConfig.h"
#include "sound/GOSoundRecorder.h"

enum {
  ID_AUDIO_RECORDER_RECORD = 0,
  ID_AUDIO_RECORDER_STOP,
  ID_AUDIO_RECORDER_RECORD_RENAME,
};

const struct ElementListEntry GOAudioRecorder::m_element_types[] = {
  {wxT("AudioRecorderRecord"), ID_AUDIO_RECORDER_RECORD, false, true},
  {wxT("AudioRecorderStop"), ID_AUDIO_RECORDER_STOP, false, true},
  {wxT("AudioRecorderRecordRename"),
   ID_AUDIO_RECORDER_RECORD_RENAME,
   false,
   true},
  {wxT(""), -1, false, false},
};

const struct ElementListEntry *GOAudioRecorder::GetButtonList() {
  return m_element_types;
}

GOAudioRecorder::GOAudioRecorder(GODefinitionFile *organfile)
  : m_organfile(organfile),
    m_recorder(NULL),
    m_RecordingTime(organfile),
    m_RecordSeconds(0),
    m_Filename(),
    m_DoRename(false) {
  CreateButtons(m_organfile);
  UpdateDisplay();
}

GOAudioRecorder::~GOAudioRecorder() { StopRecording(); }

void GOAudioRecorder::SetAudioRecorder(GOSoundRecorder *recorder) {
  StopRecording();
  m_recorder = recorder;
}

void GOAudioRecorder::Load(GOConfigReader &cfg) {
  m_button[ID_AUDIO_RECORDER_RECORD]->Init(
    cfg, wxT("AudioRecorderRecord"), _("REC"));
  m_button[ID_AUDIO_RECORDER_STOP]->Init(
    cfg, wxT("AudioRecorderStop"), _("STOP"));
  m_button[ID_AUDIO_RECORDER_RECORD_RENAME]->Init(
    cfg, wxT("AudioRecorderRecordRename"), _("REC File"));
  m_RecordingTime.Init(cfg, wxT("AudioRecordTime"), _("Audio recording time"));
}

void GOAudioRecorder::ButtonChanged(int id) {
  switch (id) {
  case ID_AUDIO_RECORDER_STOP:
    StopRecording();
    break;

  case ID_AUDIO_RECORDER_RECORD:
    StartRecording(false);
    break;

  case ID_AUDIO_RECORDER_RECORD_RENAME:
    StartRecording(true);
    break;
  }
}

GOEnclosure *GOAudioRecorder::GetEnclosure(
  const wxString &name, bool is_panel) {
  return NULL;
}

GOLabel *GOAudioRecorder::GetLabel(const wxString &name, bool is_panel) {
  if (is_panel)
    return NULL;

  if (name == wxT("AudioRecorderLabel"))
    return &m_RecordingTime;
  return NULL;
}

bool GOAudioRecorder::IsRecording() {
  return m_recorder && m_recorder->IsOpen();
}

void GOAudioRecorder::UpdateDisplay() {
  if (!IsRecording())
    m_RecordingTime.SetContent(_("-:--:--"));
  else
    m_RecordingTime.SetContent(wxString::Format(
      _("%d:%02d:%02d"),
      m_RecordSeconds / 3600,
      (m_RecordSeconds / 60) % 60,
      m_RecordSeconds % 60));
}

void GOAudioRecorder::StopRecording() {
  m_button[ID_AUDIO_RECORDER_RECORD]->Display(false);
  m_button[ID_AUDIO_RECORDER_RECORD_RENAME]->Display(false);
  m_organfile->DeleteTimer(this);
  if (!IsRecording())
    return;

  m_recorder->Close();
  if (!m_DoRename) {
    wxFileName name = m_Filename;
    GOSyncDirectory(name.GetPath());
  } else
    GOAskRenameFile(
      m_Filename,
      m_organfile->GetSettings().AudioRecorderPath(),
      _("WAV files (*.wav)|*.wav"));
  UpdateDisplay();
}

void GOAudioRecorder::StartRecording(bool rename) {
  StopRecording();
  if (!m_organfile)
    return;

  m_Filename = m_organfile->GetSettings().AudioRecorderPath()
    + wxFileName::GetPathSeparator()
    + wxDateTime::UNow().Format(_("%Y-%m-%d-%H-%M-%S.%l.wav"));
  m_DoRename = rename;

  m_recorder->Open(m_Filename);
  if (!IsRecording())
    return;

  if (m_DoRename)
    m_button[ID_AUDIO_RECORDER_RECORD_RENAME]->Display(true);
  else
    m_button[ID_AUDIO_RECORDER_RECORD]->Display(true);

  m_RecordSeconds = 0;
  UpdateDisplay();
  m_organfile->SetRelativeTimer(1000, this, 1000);
}

void GOAudioRecorder::HandleTimer() {
  m_RecordSeconds++;
  UpdateDisplay();
}
