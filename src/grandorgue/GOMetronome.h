/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMETRONOME_H
#define GOMETRONOME_H

#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"
#include "sound/GOSoundStateHandler.h"

#include "GOSaveableObject.h"
#include "GOTimerCallback.h"

class GOMidiEvent;
class GORank;
class GOOrganController;

class GOMetronome : private GOTimerCallback,
                    private GOSoundStateHandler,
                    private GOSaveableObject,
                    public GOElementCreator {
private:
  GOOrganController *m_OrganController;
  unsigned m_BPM;
  unsigned m_MeasureLength;
  unsigned m_Pos;
  bool m_Running;
  GOLabelControl m_BPMDisplay;
  GOLabelControl m_MeasureDisplay;
  GORank *m_rank;
  unsigned m_StopID;

  static const struct GOElementCreator::ButtonDefinitionEntry m_element_types[];
  const struct GOElementCreator::ButtonDefinitionEntry *
  GetButtonDefinitionList();

  void HandleTimer();

  void ButtonStateChanged(int id, bool newState) override;

  void AbortPlayback();
  void PreparePlayback();

  void Save(GOConfigWriter &cfg);

  void StartTimer();
  void StopTimer();
  void UpdateState();
  void UpdateBPM(int val);
  void UpdateMeasure(int val);

public:
  GOMetronome(GOOrganController *organController);
  virtual ~GOMetronome();

  void Load(GOConfigReader &cfg);

  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel);
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel);
};

#endif
