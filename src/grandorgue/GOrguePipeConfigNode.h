/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPECONFIGNODE_H
#define GORGUEPIPECONFIGNODE_H

#include "GOrguePipeConfig.h"
#include "GOrgueSaveableObject.h"

class GOrgueSampleStatistic;
class GOrgueStatisticCallback;

class GOrguePipeConfigNode : private GOrgueSaveableObject
{
private:
	GrandOrgueFile* m_organfile;
	GOrguePipeConfigNode* m_parent;
	GOrguePipeConfig m_PipeConfig;
	GOrgueStatisticCallback* m_StatisticCallback;
	wxString m_Name;

	void Save(GOrgueConfigWriter& cfg);

public:
	GOrguePipeConfigNode(GOrguePipeConfigNode* parent, GrandOrgueFile* organfile, GOrguePipeUpdateCallback* callback, GOrgueStatisticCallback* statistic);
	virtual ~GOrguePipeConfigNode();

	void SetParent(GOrguePipeConfigNode* parent);
	void Init(GOrgueConfigReader& cfg, wxString group, wxString prefix);
	void Load(GOrgueConfigReader& cfg, wxString group, wxString prefix);

	const wxString& GetName();
	void SetName(wxString name);

	GOrguePipeConfig& GetPipeConfig();

    // Modify Tuning / Change Tuning
	void ModifyTuning(float diff);

	float GetEffectiveAmplitude();
	float GetEffectiveGain();
	float GetEffectiveTuning();
	float GetDefaultTuning();

	unsigned GetEffectiveDelay();
    unsigned GetEffectiveReleaseTruncationLength();
	wxString GetEffectiveAudioGroup();

	unsigned GetEffectiveBitsPerSample();
	bool GetEffectiveCompress();
	unsigned GetEffectiveLoopLoad();
	unsigned GetEffectiveAttackLoad();
	unsigned GetEffectiveReleaseLoad();
	unsigned GetEffectiveChannels();

	virtual void AddChild(GOrguePipeConfigNode* node);
	virtual unsigned GetChildCount();
	virtual GOrguePipeConfigNode* GetChild(unsigned index);
	virtual GOrgueSampleStatistic GetStatistic();
};

#endif
