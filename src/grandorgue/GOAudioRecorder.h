/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIORECORDER_H
#define GOAUDIORECORDER_H

#include <wx/string.h>

#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"

#include "GOTimerCallback.h"

class GOOrganController;
class GOSoundRecorder;

class GOAudioRecorder : public GOElementCreator, private GOTimerCallback {
private:
  GOOrganController *m_OrganController;
  GOSoundRecorder *m_recorder;
  GOLabelControl m_RecordingTime;
  unsigned m_RecordSeconds;
  wxString m_Filename;
  bool m_DoRename;

  void ButtonStateChanged(int id, bool newState) override;

  void UpdateDisplay();
  void HandleTimer() override;

public:
  GOAudioRecorder(GOOrganController *organController);
  ~GOAudioRecorder();

  void SetAudioRecorder(GOSoundRecorder *recorder);

  void StartRecording(bool rename);
  bool IsRecording();
  void StopRecording();

  void Load(GOConfigReader &cfg) override;
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) override;
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel) override;
};

#endif
