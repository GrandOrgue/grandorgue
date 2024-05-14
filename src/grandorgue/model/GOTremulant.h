/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTREMULANT_H
#define GOTREMULANT_H

#include <cstdint>
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
  unsigned m_TremulantN;

  void InitSoundProvider(GOMemoryPool &pool);
  void ChangeState(bool on);
  void SetupIsToStoreInCmb() override;

  void Initialize();
  void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool);
  bool LoadCache(GOMemoryPool &pool, GOCache &cache);
  bool SaveCache(GOCacheWriter &cache) const override { return true; }
  void UpdateHash(GOHash &hash) const override {}
  const wxString &GetLoadTitle() const override { return m_Name; };

  void AbortPlayback();
  void StartPlayback();

public:
  GOTremulant(GOOrganModel &organModel);
  ~GOTremulant();
  void Load(GOConfigReader &cfg, const wxString &group, unsigned tremulantN);
  GOTremulantType GetTremulantType();

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
};

#endif /* GOTREMULANT_H_ */
