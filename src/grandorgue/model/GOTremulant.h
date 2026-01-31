/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTREMULANT_H
#define GOTREMULANT_H

#include <cstdint>
#include <wx/string.h>

#include "GOCacheObject.h"
#include "GODrawstop.h"

class GOConfigReader;
class GOConfigWriter;
class GOMemoryPool;
class GOSoundOrganInterface;
class GOSoundProvider;
struct GOSoundSampler;

typedef enum { GOSynthTrem, GOWavTrem } GOTremulantType;

class GOTremulant : public GODrawstop, private GOCacheObject {
private:
  GOSoundOrganInterface &r_sound;

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
  void OnDrawstopStateChanged(bool on) override;
  void SetupIsToStoreInCmb() override;

  void Initialize() override;
  void LoadData(const GOFileStore &fileStore, GOMemoryPool &pool) override;
  bool LoadCache(GOMemoryPool &pool, GOCache &cache) override;
  bool SaveCache(GOCacheWriter &cache) const override { return true; }
  void UpdateHash(GOHash &hash) const override {}
  const wxString &GetLoadTitle() const override { return GetName(); };

  void AbortPlayback() override;
  void StartPlayback() override;

public:
  GOTremulant(GOOrganModel &organModel);
  ~GOTremulant();
  using GODrawstop::Load; // Avoiding a compilation warning
  void Load(GOConfigReader &cfg, const wxString &group, unsigned tremulantN);
  GOTremulantType GetTremulantType();
};

#endif /* GOTREMULANT_H_ */
