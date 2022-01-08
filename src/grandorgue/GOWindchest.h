/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOWINDCHEST_H
#define GOWINDCHEST_H

#include <wx/string.h>

#include <vector>

#include "GOPipeConfigTreeNode.h"
#include "GOPlaybackStateHandler.h"

class GOConfigReader;
class GOEnclosure;
class GOPipeWindchestCallback;
class GORank;
class GOTremulant;
class GODefinitionFile;

class GOWindchest : private GOPlaybackStateHandler {
 private:
  GODefinitionFile* m_organfile;
  wxString m_Name;
  float m_Volume;
  std::vector<GOEnclosure*> m_enclosure;
  std::vector<unsigned> m_tremulant;
  std::vector<GORank*> m_ranks;
  std::vector<GOPipeWindchestCallback*> m_pipes;
  GOPipeConfigTreeNode m_PipeConfig;

  void AbortPlayback();
  void StartPlayback();
  void PreparePlayback();
  void PrepareRecording();

 public:
  GOWindchest(GODefinitionFile* organfile);

  void Init(GOConfigReader& cfg, wxString group, wxString name);
  void Load(GOConfigReader& cfg, wxString group, unsigned index);
  void UpdateTremulant(GOTremulant* tremulant);
  void UpdateVolume();
  float GetVolume();
  unsigned GetTremulantCount();
  unsigned GetTremulantId(unsigned index);
  unsigned GetRankCount();
  GORank* GetRank(unsigned index);
  void AddRank(GORank* rank);
  void AddPipe(GOPipeWindchestCallback* pipe);
  void AddEnclosure(GOEnclosure* enclosure);
  const wxString& GetName();
  GOPipeConfigNode& GetPipeConfig();
};

#endif /* GOWINDCHEST_H_ */
