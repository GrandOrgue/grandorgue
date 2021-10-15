/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOSOUNDTOUCHWORKITEM_H
#define GOSOUNDTOUCHWORKITEM_H

#include "GOSoundWorkItem.h"
#include "threading/GOMutex.h"

class GOrgueMemoryPool;

class GOSoundTouchWorkItem : public GOSoundWorkItem
{
private:
	GOrgueMemoryPool& m_Pool;
	GOMutex m_Mutex;
	bool m_Stop;

public:
	GOSoundTouchWorkItem(GOrgueMemoryPool& pool);

	unsigned GetGroup();
	unsigned GetCost();
	bool GetRepeat();
	void Run(GOSoundThread *thread = nullptr);
	void Exec();

	void Clear();
	void Reset();
};

#endif
