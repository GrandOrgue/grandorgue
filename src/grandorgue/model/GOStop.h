/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTOP_H
#define GOSTOP_H

#include <wx/string.h>

#include <vector>

#include "GODrawStop.h"

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
  void ChangeState(bool on);

  void AbortPlayback();
  void PreparePlayback();
  void StartPlayback();

public:
  GOStop(GOOrganModel &organModel, unsigned first_midi_note_number);
  GORank *GetRank(unsigned index);
  void Load(GOConfigReader &cfg, wxString group);
  void SetKey(unsigned note, unsigned velocity);
  ~GOStop(void);

  unsigned IsAuto() const;

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif /* GOSTOP_H_ */
