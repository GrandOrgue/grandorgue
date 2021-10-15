/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPIPEUPDATECALLBACK_H
#define GORGUEPIPEUPDATECALLBACK_H

class GOrguePipeUpdateCallback
{
public:
	virtual ~GOrguePipeUpdateCallback()
	{
	}

	virtual void UpdateAmplitude() = 0;
	virtual void UpdateTuning() = 0;
	virtual void UpdateReleaseTruncationLength() = 0;
	virtual void UpdateAudioGroup() = 0;
};

#endif
