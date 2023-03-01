/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTREMULANT_H
#define GOTREMULANT_H

#include <wx/string.h>

#include "GOCacheObject.h"
#include "GODrawStop.h"

class GOSoundProvider;
class GOConfigReader;
class GOConfigWriter;
class GOMemoryPool;
class GOSoundSampler;
struct IniFileEnumEntry;

typedef enum { GOSynthTrem, GOWavTrem } GOTremulantType;

class GOTremulant : public GODrawstop, private GOCacheObject {
private:
  static const struct IniFileEnumEntry m_tremulant_types[];
  GOTremulantType m_TremulantType;
  int m_Period;
  int m_StartRate;
  int m_StopRate;
  int m_AmpModDepth;
  GOSoundProvider *m_TremProvider;
  GOSoundSampler *m_PlaybackHandle;
  uint64_t m_LastStop;
  int m_SamplerGroupID;

  void InitSoundProvider(GOMemoryPool &pool);
  void ChangeState(bool on);
  void SetupCombinationState();

  void Initialize();
  void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool);
  bool LoadCache(GOMemoryPool &pool, GOCache &cache);
  bool SaveCache(GOCacheWriter &cache);
  void UpdateHash(GOHash &hash);
  const wxString &GetLoadTitle();

  void AbortPlayback();
  void StartPlayback();

public:
  GOTremulant(GOOrganController *organController);
  ~GOTremulant();
  void Load(GOConfigReader &cfg, wxString group, int sampler_group_id);
  GOTremulantType GetTremulantType();

  wxString GetMidiType();
};

#endif /* GOTREMULANT_H_ */
