/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCoupler.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"

#include "GOManual.h"
#include "GOOrganModel.h"

GOCoupler::GOCoupler(
  GOOrganModel &organModel,
  unsigned sourceManual,
  bool isVirtual,
  const GOMidiObjectContext *pContext)
  : GODrawstop(organModel, OBJECT_TYPE_COUPLER),
    m_IsVirtual(isVirtual),
    m_UnisonOff(false),
    m_CoupleToSubsequentUnisonIntermanualCouplers(false),
    m_CoupleToSubsequentUpwardIntermanualCouplers(false),
    m_CoupleToSubsequentDownwardIntermanualCouplers(false),
    m_CoupleToSubsequentUpwardIntramanualCouplers(false),
    m_CoupleToSubsequentDownwardIntramanualCouplers(false),
    m_IsNewBasMel(false),
    m_CouplerType(COUPLER_NORMAL),
    m_SourceManual(sourceManual),
    m_CouplerIndexInDest(0),
    m_DestinationManual(0),
    m_DestinationKeyshift(0),
    m_Keyshift(0),
    m_KeyVelocity(0),
    m_InternalVelocity(0),
    m_OutVelocity(0),
    m_CurrentTone(-1),
    m_LastTone(-1),
    m_FirstMidiNote(0),
    m_FirstLogicalKey(0),
    m_NumberOfKeys(127) {
  SetContext(pContext);
}

void GOCoupler::PreparePlayback() {
  GODrawstop::PreparePlayback();

  GOManual *src = r_OrganModel.GetManual(m_SourceManual);

  m_KeyVelocity.resize(src->GetLogicalKeyCount());
  std::fill(m_KeyVelocity.begin(), m_KeyVelocity.end(), 0);

  if (m_FirstMidiNote > src->GetFirstLogicalKeyMIDINoteNumber())
    m_FirstLogicalKey
      = m_FirstMidiNote - src->GetFirstLogicalKeyMIDINoteNumber();
  else
    m_FirstLogicalKey = 0;
  if (!m_UnisonOff) {
    GOManual *dest = r_OrganModel.GetManual(m_DestinationManual);

    m_InternalVelocity.resize(dest->GetLogicalKeyCount());
    std::fill(m_InternalVelocity.begin(), m_InternalVelocity.end(), 0);
    m_OutVelocity.resize(dest->GetLogicalKeyCount());
    std::fill(m_OutVelocity.begin(), m_OutVelocity.end(), 0);

    m_Keyshift = m_DestinationKeyshift + src->GetFirstLogicalKeyMIDINoteNumber()
      - dest->GetFirstLogicalKeyMIDINoteNumber();

    // Currently only the global setting option will be used. This needs to be
    // changed if a per coupler setting option is to be implemented.
    SetIsNewBasMel(r_OrganModel.GetConfig().NewBasMelBehaviour());
  }
  if (m_UnisonOff && IsEngaged())
    src->SetUnisonOff(true);
}

void GOCoupler::SetRecursive(bool isRecursive) {
  m_CoupleToSubsequentUnisonIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentUpwardIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentDownwardIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentUpwardIntramanualCouplers = isRecursive;
  m_CoupleToSubsequentDownwardIntramanualCouplers = isRecursive;
}

static const GOConfigEnum COUPLER_TYPES({
  {wxT("Normal"), GOCoupler::COUPLER_NORMAL},
  {wxT("Bass"), GOCoupler::COUPLER_BASS},
  {wxT("Melody"), GOCoupler::COUPLER_MELODY},
});

void GOCoupler::Init(
  GOConfigReader &cfg,
  const wxString &group,
  const wxString &name,
  bool unison_off,
  bool recursive,
  int keyshift,
  int dest_manual,
  GOCouplerType coupler_type) {
  m_UnisonOff = unison_off;
  m_DestinationManual = dest_manual;
  m_DestinationKeyshift = keyshift;
  SetRecursive(recursive);
  GODrawstop::Init(cfg, group, name);

  m_CouplerType = coupler_type;
  m_FirstMidiNote = 0;
  m_NumberOfKeys = 127;

  if (!m_UnisonOff)
    m_CouplerIndexInDest
      = r_OrganModel.GetManual(m_DestinationManual)->RegisterCoupler(this);
}

void GOCoupler::Load(GOConfigReader &cfg, const wxString &group) {
  m_UnisonOff
    = cfg.ReadBoolean(ODFSetting, group, wxT("UnisonOff"), true, false);
  m_DestinationManual = cfg.ReadInteger(
    ODFSetting,
    group,
    wxT("DestinationManual"),
    r_OrganModel.GetFirstManualIndex(),
    r_OrganModel.GetManualAndPedalCount(),
    !m_UnisonOff,
    0);
  m_DestinationKeyshift = cfg.ReadInteger(
    ODFSetting, group, wxT("DestinationKeyshift"), -24, 24, !m_UnisonOff, 0);
  if (m_UnisonOff) {
    SetRecursive(false);
    m_CouplerType = COUPLER_NORMAL;
    m_FirstMidiNote = 0;
    m_NumberOfKeys = 127;
    if (!r_OrganModel.GetConfig().ODFCheck()) {
      cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUnisonIntermanualCouplers"),
        false,
        false);
      cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUpwardIntermanualCouplers"),
        false,
        false);
      cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentDownwardIntermanualCouplers"),
        false,
        false);
      cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUpwardIntramanualCouplers"),
        false,
        false);
      cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentDownwardIntramanualCouplers"),
        false,
        false);
      cfg.ReadInteger(
        ODFSetting, group, wxT("FirstMIDINoteNumber"), 0, 127, false, 0);
      cfg.ReadInteger(
        ODFSetting, group, wxT("NumberOfKeys"), 0, 127, false, 127);
    }
  } else {
    m_CouplerType = (GOCouplerType)cfg.ReadEnum(
      ODFSetting,
      group,
      wxT("CouplerType"),
      COUPLER_TYPES,
      false,
      COUPLER_NORMAL);
    if (m_CouplerType == COUPLER_BASS || m_CouplerType == COUPLER_MELODY) {
      SetRecursive(false);
      if (!r_OrganModel.GetConfig().ODFCheck()) {
        cfg.ReadBoolean(
          ODFSetting,
          group,
          wxT("CoupleToSubsequentUnisonIntermanualCouplers"),
          false,
          false);
        cfg.ReadBoolean(
          ODFSetting,
          group,
          wxT("CoupleToSubsequentUpwardIntermanualCouplers"),
          false,
          false);
        cfg.ReadBoolean(
          ODFSetting,
          group,
          wxT("CoupleToSubsequentDownwardIntermanualCouplers"),
          false,
          false);
        cfg.ReadBoolean(
          ODFSetting,
          group,
          wxT("CoupleToSubsequentUpwardIntramanualCouplers"),
          false,
          false);
        cfg.ReadBoolean(
          ODFSetting,
          group,
          wxT("CoupleToSubsequentDownwardIntramanualCouplers"),
          false,
          false);
      }
    } else {
      m_CoupleToSubsequentUnisonIntermanualCouplers = cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUnisonIntermanualCouplers"),
        true,
        false);
      m_CoupleToSubsequentUpwardIntermanualCouplers = cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUpwardIntermanualCouplers"),
        true,
        false);
      m_CoupleToSubsequentDownwardIntermanualCouplers = cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentDownwardIntermanualCouplers"),
        true,
        false);
      m_CoupleToSubsequentUpwardIntramanualCouplers = cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentUpwardIntramanualCouplers"),
        true,
        false);
      m_CoupleToSubsequentDownwardIntramanualCouplers = cfg.ReadBoolean(
        ODFSetting,
        group,
        wxT("CoupleToSubsequentDownwardIntramanualCouplers"),
        true,
        false);
    }
    m_FirstMidiNote = cfg.ReadInteger(
      ODFSetting, group, wxT("FirstMIDINoteNumber"), 0, 127, false, 0);
    m_NumberOfKeys = cfg.ReadInteger(
      ODFSetting, group, wxT("NumberOfKeys"), 0, 127, false, 127);
  }
  GODrawstop::Load(cfg, group);

  if (!m_UnisonOff)
    m_CouplerIndexInDest
      = r_OrganModel.GetManual(m_DestinationManual)->RegisterCoupler(this);
}

void GOCoupler::SetupIsToStoreInCmb() {
  GODrawstop::SetupIsToStoreInCmb();
  m_IsToStoreInDivisional = m_IsToStoreInDivisional
    && (IsIntermanual() ? r_OrganModel.DivisionalsStoreIntermanualCouplers()
                        : r_OrganModel.DivisionalsStoreIntramanualCouplers());
}

void GOCoupler::SetOut(int noteNumber, unsigned velocity) {
  assert(!m_UnisonOff); // ChangeKey doesn't call SetOut for UnisonOff

  if (noteNumber < 0)
    return;
  unsigned note = noteNumber;
  if (note >= m_InternalVelocity.size())
    return;
  if (m_InternalVelocity[note] == velocity)
    return;
  m_InternalVelocity[note] = velocity;

  if (!IsEngaged())
    return;
  unsigned newstate = m_InternalVelocity[note];
  if (newstate)
    newstate--;
  m_OutVelocity[note] = newstate;

  GOManual *dest = r_OrganModel.GetManual(m_DestinationManual);

  dest->SetKeyState(note, m_OutVelocity[note], m_CouplerIndexInDest);
}

unsigned GOCoupler::GetInternalState(int noteNumber) {
  if (noteNumber < 0)
    return 0;
  unsigned note = noteNumber;
  if (note >= m_InternalVelocity.size())
    return 0;
  return m_InternalVelocity[note];
}

/**
  Returns the next pressed key after afterKey in order of coupler.
  If afterKey == -1 then returns the first pressed key.
  Returns -1 if no keys are pressed or if the coupler is neither BAS nor MEL
**/
int GOCoupler::GetNextBasMelPressedKey(int afterKey) const {
  int nextNote = -1; // no key is pressed
  const int keysCount = (int)m_KeyVelocity.size();

  if (m_CouplerType == COUPLER_BASS) {
    for (nextNote = afterKey + 1; nextNote < keysCount; nextNote++)
      if (m_KeyVelocity[nextNote] > 0)
        break;
    if (nextNote == keysCount)
      nextNote = -1;
  } else if (m_CouplerType == COUPLER_MELODY) {
    for (nextNote = (afterKey >= 0 ? afterKey : keysCount) - 1; nextNote >= 0;
         nextNote--)
      if (m_KeyVelocity[nextNote] > 0)
        break;
  }
  return nextNote;
}

/**
  Always processes every attack/release for every coupler.
  Parameter note with value 0 is actually the first key.
  A release is indicated by a velocity of 0, otherwise it's an attack.
**/
void GOCoupler::ChangeKey(int note, unsigned velocity) {
  if (m_UnisonOff)
    return;
  if (m_CouplerType == COUPLER_BASS || m_CouplerType == COUPLER_MELODY) {
    // nextNote is initially the lowest/highest currently pressed note
    int nextNote = GetNextBasMelPressedKey(-1);

    // Cancel currently coupled note if it should change
    if (m_CurrentTone != -1 && nextNote != m_CurrentTone) {
      SetOut(m_CurrentTone + m_Keyshift, 0);
      m_CurrentTone = -1;
    }

    // Couple the lowest/highest note if a matching attack is processed. If a
    // release is processed use the lowest/highest if it matches m_LastTone.
    if (
      (velocity > 0 && nextNote == note)
      || (velocity == 0 && nextNote == m_LastTone))
      m_CurrentTone = nextNote;

    // Always sends the note to be coupled if it's valid
    if (m_CurrentTone != -1)
      SetOut(m_CurrentTone + m_Keyshift, m_KeyVelocity[m_CurrentTone]);

    if (m_IsNewBasMel) {
      // In the new algorithm nextNote will now indicate the next candidate to
      // the currently lowest/highest. It's stored as m_LastTone for any attack
      // except if the currently processed attack itself is a better choice
      // which also is true if no next candidate exist i.e. is invalid.
      nextNote = GetNextBasMelPressedKey(nextNote);
      if (m_CouplerType == COUPLER_BASS) {
        if (velocity > 0)
          m_LastTone
            = (note < nextNote) ? note : (nextNote < 0 ? note : nextNote);
        else
          m_LastTone = (note < nextNote) ? -1 : nextNote;
      } else {
        if (velocity > 0)
          m_LastTone
            = (note > nextNote) ? note : (nextNote > 0 ? nextNote : note);
        else
          m_LastTone = (note > nextNote) ? -1 : nextNote;
      }
    } else {
      // In the older algorithm m_LastTone is always just the latest attack and
      // invalidated (set to -1) as soon as any release is processed.
      m_LastTone = (velocity > 0) ? note : -1;
    }
  } else
    SetOut(note + m_Keyshift, velocity);
}

void GOCoupler::SetKey(
  unsigned note,
  const std::vector<unsigned> &velocities,
  const std::vector<GOCoupler *> &couplers) {
  if (note < 0 || note >= m_KeyVelocity.size())
    return;
  if (note < m_FirstLogicalKey || note >= m_FirstLogicalKey + m_NumberOfKeys)
    return;

  assert(velocities.size() == couplers.size());
  unsigned velocity = 0;
  for (unsigned i = 0; i < velocities.size(); i++) {
    GOCoupler *prev = couplers[i];
    if (prev) {
      if (
        (prev->m_CoupleToSubsequentUnisonIntermanualCouplers
         && m_DestinationKeyshift == 0)
        || (prev->m_CoupleToSubsequentDownwardIntramanualCouplers && m_DestinationKeyshift < 0 && !IsIntermanual())
        || (prev->m_CoupleToSubsequentUpwardIntramanualCouplers && m_DestinationKeyshift > 0 && !IsIntermanual())
        || (prev->m_CoupleToSubsequentDownwardIntermanualCouplers && m_DestinationKeyshift < 0 && IsIntermanual())
        || (prev->m_CoupleToSubsequentUpwardIntermanualCouplers && m_DestinationKeyshift > 0 && IsIntermanual()))
        ;
      else
        continue;
    }
    if (velocities[i] > velocity)
      velocity = velocities[i];
  }
  if (m_KeyVelocity[note] == velocity)
    return;
  m_KeyVelocity[note] = velocity;
  ChangeKey(note, velocity);
}

void GOCoupler::OnDrawstopStateChanged(bool on) {
  if (m_UnisonOff)
    r_OrganModel.GetManual(m_SourceManual)->SetUnisonOff(on);
  else {
    GOManual *dest = r_OrganModel.GetManual(m_DestinationManual);

    for (unsigned i = 0; i < m_InternalVelocity.size(); i++) {
      unsigned newstate = on ? m_InternalVelocity[i] : 0;
      if (newstate > 0)
        newstate--;
      if (m_OutVelocity[i] != newstate) {
        m_OutVelocity[i] = newstate;
        dest->SetKeyState(i, m_OutVelocity[i], m_CouplerIndexInDest);
      }
    }
  }
}

void GOCoupler::RefreshState() {
  bool on = IsEngaged();

  if (m_UnisonOff)
    r_OrganModel.GetManual(m_SourceManual)->SetUnisonOff(on);
  else {
    GOManual *dest = r_OrganModel.GetManual(m_DestinationManual);

    for (unsigned l = m_OutVelocity.size(), i = 0; i < l; i++)
      // check if the key is set. Otherwise propagating is not necessary
      if (m_OutVelocity[i])
        dest->PropagateKeyToCouplers(i);
  }
}

bool GOCoupler::IsIntermanual() {
  return m_SourceManual != m_DestinationManual;
}

bool GOCoupler::IsUnisonOff() { return m_UnisonOff; }
