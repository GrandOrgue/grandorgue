/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIPLAYER_H
#define GOMIDIPLAYER_H

#include <wx/string.h>
#include <wx/timer.h>

#include <vector>

#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"
#include "midi/GOMidiPlayerContent.h"

#include "GOTime.h"
#include "GOTimerCallback.h"

class GOMidiEvent;
class GOMidiFileReader;
class GOOrganController;

class GOMidiPlayer : public GOElementCreator, private GOTimerCallback {
private:
  GOOrganController *m_OrganController;
  GOMidiPlayerContent m_content;
  GOLabelControl m_PlayingTime;
  GOTime m_Start;
  unsigned m_PlayingSeconds;
  float m_Speed;
  bool m_IsPlaying;
  bool m_Pause;
  unsigned m_DeviceID;

  static const struct GOElementCreator::ButtonDefinitionEntry m_element_types[];
  const struct GOElementCreator::ButtonDefinitionEntry *
  GetButtonDefinitionList();

  void ButtonStateChanged(int id, bool newState) override;

  void UpdateDisplay();
  void HandleTimer();

public:
  GOMidiPlayer(GOOrganController *organController);
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
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel);
};

#endif
