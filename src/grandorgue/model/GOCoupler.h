/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCOUPLER_H
#define GOCOUPLER_H

#include <vector>

#include "GODrawstop.h"

class GOConfigReader;
class GOConfigWriter;

class GOCoupler : public GODrawstop {
public:
  typedef enum { COUPLER_NORMAL, COUPLER_BASS, COUPLER_MELODY } GOCouplerType;

private:
  bool m_IsVirtual;
  bool m_UnisonOff;
  bool m_CoupleToSubsequentUnisonIntermanualCouplers;
  bool m_CoupleToSubsequentUpwardIntermanualCouplers;
  bool m_CoupleToSubsequentDownwardIntermanualCouplers;
  bool m_CoupleToSubsequentUpwardIntramanualCouplers;
  bool m_CoupleToSubsequentDownwardIntramanualCouplers;
  bool m_IsNewBasMel;
  GOCouplerType m_CouplerType;
  unsigned m_SourceManual;
  unsigned m_CouplerIndexInDest;
  unsigned m_DestinationManual;
  int m_DestinationKeyshift;
  int m_Keyshift;
  /* Input state to the coupler */
  std::vector<unsigned> m_KeyVelocity;
  /* Intended output state always assuming the coupler is ON */
  std::vector<unsigned> m_InternalVelocity;
  /* Current ouput state */
  std::vector<unsigned> m_OutVelocity;
  int m_CurrentTone;
  int m_LastTone;
  int m_FirstMidiNote;
  unsigned m_FirstLogicalKey;
  unsigned m_NumberOfKeys;

  void ChangeKey(int note, unsigned velocity);
  void SetOut(int note, unsigned velocity);
  unsigned GetInternalState(int note);
  void OnDrawstopStateChanged(bool on) override;
  void SetupIsToStoreInCmb() override;

  void PreparePlayback() override;

public:
  GOCoupler(
    GOOrganModel &organModel,
    unsigned sourceManual,
    bool isVirtual,
    const GOMidiObjectContext *pContext);

  bool IsVirtual() const { return m_IsVirtual; }
  bool IsRecursive() const {
    return m_CoupleToSubsequentUnisonIntermanualCouplers;
  }
  void SetRecursive(bool isRecursive);
  void SetIsNewBasMel(bool isNew) { m_IsNewBasMel = isNew; }
  bool IsNewBasMel() const { return m_IsNewBasMel; }

  using GODrawstop::Init; // for avoiding a compilation warning
  void Init(
    GOConfigReader &cfg,
    const wxString &group,
    const wxString &name,
    bool unison_off,
    bool recursive,
    int keyshift,
    int dest_manual,
    GOCouplerType coupler_type);
  void Load(GOConfigReader &cfg, const wxString &group) override;

  // send key states for all chained couplers
  void RefreshState();

  void SetKey(
    unsigned note,
    const std::vector<unsigned> &velocities,
    const std::vector<GOCoupler *> &couplers);
  bool IsIntermanual();
  bool IsUnisonOff();
};

#endif
