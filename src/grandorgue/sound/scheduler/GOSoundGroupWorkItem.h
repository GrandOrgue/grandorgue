/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDGROUPWORKITEM_H
#define GOSOUNDGROUPWORKITEM_H

#include "sound/GOSoundBufferItem.h"
#include "sound/GOSoundSamplerList.h"
#include "sound/scheduler/GOSoundWorkItem.h"
#include "threading/GOCondition.h"
#include "threading/GOMutex.h"
#include "GOSoundThread.h"

class GOSoundEngine;

class GOSoundGroupWorkItem : public GOSoundWorkItem, public GOSoundBufferItem
{
private:
	GOSoundEngine& m_engine;
	GOSoundSamplerList m_Active;
	GOSoundSamplerList m_Release;
	GOMutex m_Mutex;
	GOCondition m_Condition;
	unsigned m_ActiveCount;
	unsigned m_Done;
	volatile bool m_Stop;

	void ProcessList(GOSoundSamplerList& list, float* output_buffer);
	void ProcessReleaseList(GOSoundSamplerList& list, float* output_buffer);

public:
	GOSoundGroupWorkItem(GOSoundEngine& sound_engine, unsigned samples_per_buffer);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run(GOSoundThread *pThread = nullptr);
	void Exec();
	void Finish(bool stop, GOSoundThread *pThread = nullptr);

	void Reset();
	void Clear();
	void Add(GOSoundSampler* sampler);
};

#endif
