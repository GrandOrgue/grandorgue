/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDOUTPUTWORKITEM_H
#define GOSOUNDOUTPUTWORKITEM_H

#include "GOSoundBufferItem.h"
#include "GOSoundWorkItem.h"
#include "threading/GOMutex.h"
#include <vector>

class GOSoundReverb;
class GOrgueSettings;

class GOSoundOutputWorkItem : public GOSoundWorkItem, public GOSoundBufferItem
{
private:
	std::vector<float> m_ScaleFactors;
	std::vector<GOSoundBufferItem*> m_Outputs;
	unsigned m_OutputCount;
	std::vector<float> m_MeterInfo;
	GOSoundReverb* m_Reverb;
	GOMutex m_Mutex;
	unsigned m_Done;
	volatile bool m_Stop;

public:
	GOSoundOutputWorkItem(unsigned channels, std::vector<float> scale_factors, unsigned samples_per_buffer);
	~GOSoundOutputWorkItem();

	void SetOutputs(std::vector<GOSoundBufferItem*> outputs);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run(GOSoundThread *pThread = nullptr);
	void Exec();
	void Finish(bool stop, GOSoundThread *pThread = nullptr);

	void Clear();
	void Reset();

	void SetupReverb(GOrgueSettings& settings);

	const std::vector<float>& GetMeterInfo();
	void ResetMeterInfo();
};

#endif
