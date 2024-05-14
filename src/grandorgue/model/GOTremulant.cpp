/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTremulant.h"

#include <wx/intl.h>

#include "config/GOConfigReader.h"
#include "model/GOOrganModel.h"
#include "sound/GOSoundEngine.h"
#include "sound/GOSoundProviderSynthedTrem.h"

#define DELETE_AND_NULL(x)                                                     \
  do {                                                                         \
    if (x) {                                                                   \
      delete x;                                                                \
      x = NULL;                                                                \
    }                                                                          \
  } while (0)

const struct IniFileEnumEntry GOTremulant::m_tremulant_types[] = {
  {wxT("Synth"), GOSynthTrem},
  {wxT("Wave"), GOWavTrem},
};

GOTremulant::GOTremulant(GOOrganModel &organModel)
  : GODrawstop(organModel),
    m_TremulantType(GOSynthTrem),
    m_Period(0),
    m_StartRate(0),
    m_StopRate(0),
    m_AmpModDepth(0),
    m_TremProvider(NULL),
    m_PlaybackHandle(0),
    m_LastStop(0),
    m_TremulantN(0) {}

GOTremulant::~GOTremulant() { DELETE_AND_NULL(m_TremProvider); }

void GOTremulant::Initialize() {}

void GOTremulant::LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) {
  InitSoundProvider(pool);
}

bool GOTremulant::LoadCache(GOMemoryPool &pool, GOCache &cache) {
  InitSoundProvider(pool);
  return true;
}

void GOTremulant::Load(
  GOConfigReader &cfg, const wxString &group, unsigned tremulantN) {
  SetGroupAndPrefix(group, wxEmptyString);
  m_TremulantType = (GOTremulantType)cfg.ReadEnum(
    ODFSetting,
    group,
    wxT("TremulantType"),
    m_tremulant_types,
    sizeof(m_tremulant_types) / sizeof(m_tremulant_types[0]),
    false,
    GOSynthTrem);
  if (m_TremulantType == GOSynthTrem) {
    m_TremProvider = new GOSoundProviderSynthedTrem();
    m_Period = cfg.ReadLong(ODFSetting, group, wxT("Period"), 32, 441000);
    m_StartRate = cfg.ReadInteger(ODFSetting, group, wxT("StartRate"), 1, 100);
    m_StopRate = cfg.ReadInteger(ODFSetting, group, wxT("StopRate"), 1, 100);
    m_AmpModDepth
      = cfg.ReadInteger(ODFSetting, group, wxT("AmpModDepth"), 1, 100);
    m_TremulantN = tremulantN;
    m_PlaybackHandle = 0;
  }
  GODrawstop::Load(cfg, group);
  r_OrganModel.RegisterCacheObject(this);
}

void GOTremulant::SetupIsToStoreInCmb() {
  GODrawstop::SetupIsToStoreInCmb();
  m_IsToStoreInDivisional
    = m_IsToStoreInDivisional && r_OrganModel.DivisionalsStoreTremulants();
}

void GOTremulant::InitSoundProvider(GOMemoryPool &pool) {
  if (m_TremulantType == GOSynthTrem) {
    ((GOSoundProviderSynthedTrem *)m_TremProvider)
      ->Create(pool, m_Period, m_StartRate, m_StopRate, m_AmpModDepth);
    assert(!m_TremProvider->IsOneshot());
  }
}

void GOTremulant::ChangeState(bool on) {
  if (m_TremulantType == GOSynthTrem) {
    GOSoundEngine *pSoundEngine = GetSoundEngine();

    if (on) {
      assert(m_TremulantN > 0);
      m_PlaybackHandle = pSoundEngine ? pSoundEngine->StartTremulantSample(
                           m_TremProvider, m_TremulantN, m_LastStop)
                                      : nullptr;
      on = (m_PlaybackHandle != nullptr);
    } else {
      assert(m_PlaybackHandle);
      m_LastStop = pSoundEngine
        ? pSoundEngine->StopSample(m_TremProvider, m_PlaybackHandle)
        : 0;
      m_PlaybackHandle = NULL;
    }
  }
  if (m_TremulantType == GOWavTrem) {
    r_OrganModel.UpdateTremulant(this);
  }
}

void GOTremulant::AbortPlayback() {
  m_PlaybackHandle = NULL;
  m_LastStop = 0;
  GOButtonControl::AbortPlayback();
}

void GOTremulant::StartPlayback() {
  GODrawstop::StartPlayback();

  if (IsActive() && m_TremulantType == GOSynthTrem) {
    GOSoundEngine *pSoundEngine = GetSoundEngine();

    assert(m_TremulantN > 0);
    m_PlaybackHandle = pSoundEngine
      ? pSoundEngine->StartTremulantSample(m_TremProvider, m_TremulantN, 0)
      : nullptr;
  }
  if (m_TremulantType == GOWavTrem) {
    r_OrganModel.UpdateTremulant(this);
  }
}

GOTremulantType GOTremulant::GetTremulantType() { return m_TremulantType; }

const wxString WX_MIDI_TYPE_CODE = wxT("Tremulant");
const wxString WX_MIDI_TYPE = _("Tremulant");

const wxString &GOTremulant::GetMidiTypeCode() const {
  return WX_MIDI_TYPE_CODE;
}

const wxString &GOTremulant::GetMidiType() const { return WX_MIDI_TYPE; }
