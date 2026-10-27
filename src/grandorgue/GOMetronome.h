/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMETRONOME_H
#define GOMETRONOME_H

#include <memory>

#include "control/GOElementCreator.h"
#include "control/GOLabelControl.h"
#include "model/GOOrganLifecycleListener.h"

#include "GOSaveableObject.h"
#include "GOTimerCallback.h"

class GOConfigReader;
class GOMidiEvent;
class GOOrganController;
class GOOrganModel;
class GORank;
class GOWindchest;

class GOMetronome : private GOTimerCallback,
                    private GOOrganLifecycleListener,
                    private GOSaveableObject,
                    public GOElementCreator {
public:
  enum {
    ID_METRONOME_ON = 0,
    ID_METRONOME_MEASURE_P1,
    ID_METRONOME_MEASURE_M1,
    ID_METRONOME_BEAT_P1,
    ID_METRONOME_BEAT_M1,
    ID_METRONOME_BEAT_P10,
    ID_METRONOME_BEAT_M10,
  };
  static const ButtonDefinitionEntry *const P_BUTTON_DEFS;

private:
  GOOrganController *m_OrganController;
  /**
   * The number (1-based, as returned by GOOrganModel::AddWindchest()) of the
   * windchest the metronome's rank and pipes sound through. The windchest is
   * created by createWindchest() and registered by the caller before this
   * object exists.
   */
  unsigned m_WindchestN;
  unsigned m_BPM;
  unsigned m_MeasureLength;
  unsigned m_Pos;
  bool m_Running;
  GOLabelControl m_BPMDisplay;
  GOLabelControl m_MeasureDisplay;
  GORank *m_rank;
  unsigned m_StopID;

  const wxString &GetSampleFilePath(bool isFirst);

  void HandleTimer() override;

  void ButtonStateChanged(int id, bool newState) override;

  void AbortPlayback() override;
  void PreparePlayback() override;

  void Save(GOConfigWriter &cfg) override;

  void StartTimer();
  void StopTimer();
  void UpdateState();
  void UpdateBPM(int val);
  void UpdateMeasure(int val);

public:
  /**
   * Creates, but does not register, the windchest the metronome plays its
   * sounds through. It is a separate static step because the windchest has
   * to exist before the internal enclosures are created, i.e. earlier than
   * the element creators are loaded. The caller adds it to the model with
   * AddWindchest() and passes the resulting number to the constructor.
   * @param cfg the config reader of the organ being loaded
   * @param organModel the model the windchest will belong to
   * @return the new windchest, ownership goes to the caller
   */
  static std::unique_ptr<GOWindchest> createWindchest(
    GOConfigReader &cfg, GOOrganModel &organModel);

  /**
   * @param organController the organ controller the metronome belongs to
   * @param windchestN the number (1-based) of the metronome's windchest,
   *   which must already be added to the model: the result of AddWindchest()
   *   for the windchest from createWindchest()
   */
  GOMetronome(GOOrganController *organController, unsigned windchestN);
  virtual ~GOMetronome();

  void Load(GOConfigReader &cfg) override;

  GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) override;
  GOLabelControl *GetLabelControl(const wxString &name, bool is_panel) override;
};

#endif
