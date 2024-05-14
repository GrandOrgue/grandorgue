/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOCoupler.h"

#include <wx/intl.h>

#include "config/GOConfig.h"
#include "config/GOConfigReader.h"

#include "GOManual.h"
#include "GOOrganModel.h"

GOCoupler::GOCoupler(GOOrganModel &organModel, unsigned sourceManual)
  : GODrawstop(organModel),
    m_UnisonOff(false),
    m_CoupleToSubsequentUnisonIntermanualCouplers(false),
    m_CoupleToSubsequentUpwardIntermanualCouplers(false),
    m_CoupleToSubsequentDownwardIntermanualCouplers(false),
    m_CoupleToSubsequentUpwardIntramanualCouplers(false),
    m_CoupleToSubsequentDownwardIntramanualCouplers(false),
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
    m_NumberOfKeys(127) {}

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
  }
  if (m_UnisonOff && IsActive())
    src->SetUnisonOff(true);
}

void GOCoupler::SetRecursive(bool isRecursive) {
  m_CoupleToSubsequentUnisonIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentUpwardIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentDownwardIntermanualCouplers = isRecursive;
  m_CoupleToSubsequentUpwardIntramanualCouplers = isRecursive;
  m_CoupleToSubsequentDownwardIntramanualCouplers = isRecursive;
}

const struct IniFileEnumEntry GOCoupler::m_coupler_types[] = {
  {wxT("Normal"), COUPLER_NORMAL},
  {wxT("Bass"), COUPLER_BASS},
  {wxT("Melody"), COUPLER_MELODY},
};

void GOCoupler::Init(
  GOConfigReader &cfg,
  wxString group,
  wxString name,
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

void GOCoupler::Load(GOConfigReader &cfg, wxString group) {
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
      m_coupler_types,
      sizeof(m_coupler_types) / sizeof(m_coupler_types[0]),
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

  if (!IsActive())
    return;
  unsigned newstate = m_InternalVelocity[note];
  if (newstate)
    newstate--;
  m_OutVelocity[note] = newstate;

  GOManual *dest = r_OrganModel.GetManual(m_DestinationManual);

  dest->SetKey(note, m_OutVelocity[note], m_CouplerIndexInDest);
}

unsigned GOCoupler::GetInternalState(int noteNumber) {
  if (noteNumber < 0)
    return 0;
  unsigned note = noteNumber;
  if (note >= m_InternalVelocity.size())
    return 0;
  return m_InternalVelocity[note];
}

void GOCoupler::ChangeKey(int note, unsigned velocity) {
  if (m_UnisonOff)
    return;
  if (m_CouplerType == COUPLER_BASS || m_CouplerType == COUPLER_MELODY) {
    int nextNote = -1;
    if (m_CouplerType == COUPLER_BASS) {
      for (nextNote = 0; nextNote < (int)m_KeyVelocity.size(); nextNote++)
        if (m_KeyVelocity[nextNote] > 0)
          break;
      if (nextNote == (int)m_KeyVelocity.size())
        nextNote = -1;
    } else {
      for (nextNote = m_KeyVelocity.size() - 1; nextNote >= 0; nextNote--)
        if (m_KeyVelocity[nextNote] > 0)
          break;
    }

    if (m_CurrentTone != -1 && nextNote != m_CurrentTone) {
      SetOut(m_CurrentTone + m_Keyshift, 0);
      m_CurrentTone = -1;
    }

    if (((velocity > 0 && nextNote == note)
         || (velocity == 0 && nextNote == m_LastTone)))
      m_CurrentTone = nextNote;

    if (m_CurrentTone != -1)
      SetOut(m_CurrentTone + m_Keyshift, m_KeyVelocity[m_CurrentTone]);

    if (velocity > 0)
      m_LastTone = note;
    else
      m_LastTone = -1;
    return;
  }
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

void GOCoupler::ChangeState(bool on) {
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
        dest->SetKey(i, m_OutVelocity[i], m_CouplerIndexInDest);
      }
    }
  }
}

void GOCoupler::RefreshState() {
  bool on = IsActive();

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

const wxString WX_MIDI_TYPE_CODE = wxT("Coupler");
const wxString WX_MIDI_TYPE = _("Coupler");

const wxString &GOCoupler::GetMidiTypeCode() const { return WX_MIDI_TYPE_CODE; }

const wxString &GOCoupler::GetMidiType() const { return WX_MIDI_TYPE; }
