/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREFERENCEPIPE_H
#define GOREFERENCEPIPE_H

#include "GOCacheObject.h"
#include "GOPipe.h"

class GOModel;

class GOReferencePipe : public GOPipe, private GOCacheObject {
private:
  GOModel *m_model;
  GOPipe *m_Reference;
  unsigned m_ReferenceID;
  wxString m_Filename;

  void Initialize();
  void LoadData(GOMemoryPool &pool) override {}
  bool LoadCache(GOMemoryPool &pool, GOCache &cache) override { return true; }
  bool SaveCache(GOCacheWriter &cache) override { return true; }
  void UpdateHash(GOHash &hash);
  const wxString &GetLoadTitle();

  void Change(unsigned velocity, unsigned old_velocity) override;

public:
  GOReferencePipe(GOModel *model, GORank *rank, unsigned midi_key_number);

  void Load(GOConfigReader &cfg, wxString group, wxString prefix);
};

#endif
