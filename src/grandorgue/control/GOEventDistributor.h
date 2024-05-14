/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOEVENTDISTRIBUTOR_H
#define GOEVENTDISTRIBUTOR_H

#include <vector>

class GOConfigReader;
class GOConfigWriter;
class GOEventHandlerList;
class GOHash;
class GOMidiEvent;
class GOSoundEngine;

class GOEventDistributor {
private:
  GOEventHandlerList *p_model;

protected:
  void SendMidi(const GOMidiEvent &event);

  void ReadCombinations(GOConfigReader &cfg);
  void Save(GOConfigWriter &cfg);

  void ResolveReferences();
  void UpdateHash(GOHash &hash);

  void PreparePlayback(GOSoundEngine *pSoundEngine);
  void StartPlayback();
  void AbortPlayback();
  void PrepareRecording();

public:
  GOEventDistributor(GOEventHandlerList *pModel) { p_model = pModel; }
  ~GOEventDistributor() { p_model = nullptr; }

  void HandleKey(int key);
};

#endif
