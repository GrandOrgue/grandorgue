/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTOP_H
#define GOSTOP_H

#include <wx/string.h>

#include <vector>

#include "GODrawstop.h"

class GORank;

class GOStop : public GODrawstop {
private:
  typedef struct {
    GORank *Rank;
    unsigned StopID;
    unsigned FirstAccessibleKeyNumber;
    unsigned FirstPipeNumber;
    unsigned PipeCount;
  } RankInfo;
  std::vector<RankInfo> m_RankInfo;
  std::vector<unsigned> m_KeyVelocity;
  unsigned m_FirstMidiNoteNumber;
  unsigned m_FirstAccessiblePipeLogicalKeyNumber;
  unsigned m_NumberOfAccessiblePipes;

  void SetRankKey(unsigned key, unsigned velocity);
  void OnDrawstopStateChanged(bool on) override;

  void AbortPlayback() override;
  void PreparePlayback() override;
  void StartPlayback() override;

public:
  GOStop(
    GOOrganModel &organModel,
    unsigned first_midi_note_number,
    GOMidiObjectContext *pContext);
  GORank *GetRank(unsigned index);
  void Load(GOConfigReader &cfg, const wxString &group) override;
  void SetKey(unsigned note, unsigned velocity);
  ~GOStop(void);

  unsigned IsAuto() const;
};

#endif /* GOSTOP_H_ */
