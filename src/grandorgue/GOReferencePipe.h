/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOREFERENCEPIPE_H
#define GOREFERENCEPIPE_H

#include "GOCacheObject.h"
#include "GOPipe.h"

class GOReferencePipe : public GOPipe, private GOCacheObject {
 private:
  GOPipe* m_Reference;
  unsigned m_ReferenceID;
  wxString m_Filename;

  void Initialize();
  void LoadData();
  bool LoadCache(GOCache& cache);
  bool SaveCache(GOCacheWriter& cache);
  void UpdateHash(GOHash& hash);
  const wxString& GetLoadTitle();

  void Change(unsigned velocity, unsigned old_velocity);

 public:
  GOReferencePipe(GODefinitionFile* organfile, GORank* rank,
                  unsigned midi_key_number);

  void Load(GOConfigReader& cfg, wxString group, wxString prefix);
};

#endif
