/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOAUDIORECORDER_H
#define GOAUDIORECORDER_H

#include <wx/string.h>

#include "control/GOElementCreator.h"

#include "GOLabel.h"
#include "GOTimerCallback.h"

class GODefinitionFile;
class GOSoundRecorder;

class GOAudioRecorder : public GOElementCreator, private GOTimerCallback {
private:
  GODefinitionFile *m_organfile;
  GOSoundRecorder *m_recorder;
  GOLabel m_RecordingTime;
  unsigned m_RecordSeconds;
  wxString m_Filename;
  bool m_DoRename;

  static const struct ElementListEntry m_element_types[];
  const struct ElementListEntry *GetButtonList();

  void ButtonChanged(int id);

  void UpdateDisplay();
  void HandleTimer();

public:
  GOAudioRecorder(GODefinitionFile *organfile);
  ~GOAudioRecorder();

  void SetAudioRecorder(GOSoundRecorder *recorder);

  void StartRecording(bool rename);
  bool IsRecording();
  void StopRecording();

  void Load(GOConfigReader &cfg);
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabel *GetLabel(const wxString &name, bool is_panel);
};

#endif
