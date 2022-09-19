/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPLAYER_H
#define GOMIDIPLAYER_H

#include <wx/string.h>
#include <wx/timer.h>

#include <vector>

#include "control/GOElementCreator.h"
#include "midi/GOMidiPlayerContent.h"

#include "GOLabel.h"
#include "GOTime.h"
#include "GOTimerCallback.h"

class GOMidiEvent;
class GOMidiFileReader;
class GODefinitionFile;

class GOMidiPlayer : public GOElementCreator, private GOTimerCallback {
private:
  GODefinitionFile *m_organfile;
  GOMidiPlayerContent m_content;
  GOLabel m_PlayingTime;
  GOTime m_Start;
  unsigned m_PlayingSeconds;
  float m_Speed;
  bool m_IsPlaying;
  bool m_Pause;
  unsigned m_DeviceID;

  static const struct GOElementCreator::ButtonDefinitionEntry m_element_types[];
  const struct GOElementCreator::ButtonDefinitionEntry *
  GetButtonDefinitionList();

  void ButtonChanged(int id);

  void UpdateDisplay();
  void HandleTimer();

public:
  GOMidiPlayer(GODefinitionFile *organfile);
  ~GOMidiPlayer();

  void Clear();
  void LoadFile(const wxString &filename, unsigned manuals, bool pedal);
  bool IsLoaded();

  void Play();
  void Pause();
  void StopPlaying();
  bool IsPlaying();

  void Load(GOConfigReader &cfg);
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabel *GetLabel(const wxString &name, bool is_panel);
};

#endif
