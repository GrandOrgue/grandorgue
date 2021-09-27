/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEWINDCHEST_H
#define GORGUEWINDCHEST_H

#include "GOrguePipeConfigTreeNode.h"
#include "GOrguePlaybackStateHandler.h"
#include <wx/string.h>
#include <vector>

class GOrgueConfigReader;
class GOrgueEnclosure;
class GOrguePipeWindchestCallback;
class GOrgueRank;
class GOrgueTremulant;
class GrandOrgueFile;

class GOrgueWindchest : private GOrguePlaybackStateHandler
{
private:
	GrandOrgueFile* m_organfile;
	wxString m_Name;
	float m_Volume;
	std::vector<GOrgueEnclosure*> m_enclosure;
	std::vector<unsigned> m_tremulant;
	std::vector<GOrgueRank*> m_ranks;
	std::vector<GOrguePipeWindchestCallback*> m_pipes;
	GOrguePipeConfigTreeNode m_PipeConfig;

	void AbortPlayback();
	void StartPlayback();
	void PreparePlayback();
	void PrepareRecording();

public:
	GOrgueWindchest(GrandOrgueFile* organfile);

	void Init(GOrgueConfigReader& cfg, wxString group, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group, unsigned index);
	void UpdateTremulant(GOrgueTremulant* tremulant);
	void UpdateVolume();
	float GetVolume();
	unsigned GetTremulantCount();
	unsigned GetTremulantId(unsigned index);
	unsigned GetRankCount();
	GOrgueRank* GetRank(unsigned index);
	void AddRank(GOrgueRank* rank);
	void AddPipe(GOrguePipeWindchestCallback* pipe);
	void AddEnclosure(GOrgueEnclosure* enclosure);
	const wxString& GetName();
	GOrguePipeConfigNode& GetPipeConfig();
};

#endif /* GORGUEWINDCHEST_H_ */
