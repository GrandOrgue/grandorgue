/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
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

class GOMidi;
class GOMidiMap;
class GOMidiEvent;
class GOMidiFileReader;
class GOOrganController;
class GOTimer;

class GOMidiPlayer : public GOElementCreator, private GOTimerCallback {
private:
  GOMidiMap &r_MidiMap;
  GOTimer &r_timer;
  GOMidi *p_midi;
  GOMidiPlayerContent m_content;
  GOLabelControl m_PlayingTime;
  GOTime m_Start;
  unsigned m_PlayingSeconds;
  float m_Speed;
  bool m_IsPlaying;
  bool m_Pause;
  unsigned m_DeviceID;

  void ButtonStateChanged(int id, bool newState) override;

  void UpdateDisplay();

  /**
   * Set the buttons and the display to the initial state
   */
  void ResetUI();
  /**
   * Send midi event to the organ
   * @param event the event to process
   */
  void PlayMidiEvent(const GOMidiEvent &e);
  void HandleTimer() override;

public:
  GOMidiPlayer(GOOrganController *organController);
  ~GOMidiPlayer();

  /**
   * Set up for playing any midi
   * @param pMidi - a pointer to the midi engine
   */
  void Setup(GOMidi *pMidi) { p_midi = pMidi; }

  void LoadFile(const wxString &filename, unsigned manuals, bool pedal);
  bool IsLoaded();

  void Play();
  void Pause();
  void StopPlaying();
  bool IsPlaying();

  void Load(GOConfigReader &cfg) override;
  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) override;
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel) override;

  /**
   * Clean up. Playing will be impossible until Setup is called
   */
  void Cleanup();
};

#endif
