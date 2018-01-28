/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2018 GrandOrgue contributors (see AUTHORS)
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

#include "GOrgueAudioRecorder.h"

#include "GOSoundRecorder.h"
#include "GOrgueEvent.h"
#include "GOrguePath.h"
#include "GOrgueSetterButton.h"
#include "GOrgueSettings.h"
#include "GrandOrgueFile.h"
#include <wx/filename.h>
#include <wx/intl.h>

enum {
	ID_AUDIO_RECORDER_RECORD = 0,
	ID_AUDIO_RECORDER_STOP,
	ID_AUDIO_RECORDER_RECORD_RENAME,
};

const struct ElementListEntry GOrgueAudioRecorder::m_element_types[] = {
	{ wxT("AudioRecorderRecord"), ID_AUDIO_RECORDER_RECORD, false, true },
	{ wxT("AudioRecorderStop"), ID_AUDIO_RECORDER_STOP, false, true },
	{ wxT("AudioRecorderRecordRename"), ID_AUDIO_RECORDER_RECORD_RENAME, false, true },
	{ wxT(""), -1, false, false },
};

const struct ElementListEntry* GOrgueAudioRecorder::GetButtonList()
{
	return m_element_types;
}

GOrgueAudioRecorder::GOrgueAudioRecorder(GrandOrgueFile* organfile) :
	m_organfile(organfile),
	m_recorder(NULL),
	m_RecordingTime(organfile),
	m_RecordSeconds(0),
	m_Filename(),
	m_DoRename(false)
{
	CreateButtons(m_organfile);
	UpdateDisplay();
}

GOrgueAudioRecorder::~GOrgueAudioRecorder()
{
	StopRecording();
}

void GOrgueAudioRecorder::SetAudioRecorder(GOSoundRecorder* recorder)
{
	StopRecording();
	m_recorder = recorder;
}

void GOrgueAudioRecorder::Load(GOrgueConfigReader& cfg)
{
	m_button[ID_AUDIO_RECORDER_RECORD]->Init(cfg, wxT("AudioRecorderRecord"), _("REC"));
	m_button[ID_AUDIO_RECORDER_STOP]->Init(cfg, wxT("AudioRecorderStop"), _("STOP"));
	m_button[ID_AUDIO_RECORDER_RECORD_RENAME]->Init(cfg, wxT("AudioRecorderRecordRename"), _("REC File"));
}

void GOrgueAudioRecorder::ButtonChanged(int id)
{
	switch(id)
	{
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

GOrgueEnclosure* GOrgueAudioRecorder::GetEnclosure(const wxString& name, bool is_panel)
{
	return NULL;
}

GOrgueLabel* GOrgueAudioRecorder::GetLabel(const wxString& name, bool is_panel)
{
	if (is_panel)
		return NULL;

	if (name == wxT("AudioRecorderLabel"))
		return &m_RecordingTime;
	return NULL;
}

bool GOrgueAudioRecorder::IsRecording()
{
	return m_recorder && m_recorder->IsOpen();
}

void GOrgueAudioRecorder::UpdateDisplay()
{
	if (!IsRecording())
		m_RecordingTime.SetName(_("-:--:--"));
	else
		m_RecordingTime.SetName(wxString::Format(_("%d:%02d:%02d"), m_RecordSeconds / 3600, (m_RecordSeconds / 60) % 60, m_RecordSeconds % 60));
}

void GOrgueAudioRecorder::StopRecording()
{
	m_button[ID_AUDIO_RECORDER_RECORD]->Display(false);
	m_button[ID_AUDIO_RECORDER_RECORD_RENAME]->Display(false);
	m_organfile->DeleteTimer(this);
	if (!IsRecording())
		return;

	m_recorder->Close();
	if (!m_DoRename)
	{
		wxFileName name = m_Filename;
		GOSyncDirectory(name.GetPath());
	}
	else
		GOAskRenameFile(m_Filename, m_organfile->GetSettings().AudioRecorderPath(),_("WAV files (*.wav)|*.wav"));
	UpdateDisplay();
}

void GOrgueAudioRecorder::StartRecording(bool rename)
{
	StopRecording();
	if (!m_organfile)
		return;

	m_Filename = m_organfile->GetSettings().AudioRecorderPath() + wxFileName::GetPathSeparator() + wxDateTime::UNow().Format(_("%Y-%m-%d-%H-%M-%S.%l.wav"));
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

void GOrgueAudioRecorder::HandleTimer()
{
	m_RecordSeconds++;
	UpdateDisplay();
}
