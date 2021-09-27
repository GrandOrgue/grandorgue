/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEPLAYBACKSTATEHANDLER_H
#define GORGUEPLAYBACKSTATEHANDLER_H

class GOrguePlaybackStateHandler
{
public:
	virtual ~GOrguePlaybackStateHandler()
	{
	}

	virtual void AbortPlayback() = 0;
	virtual void PreparePlayback() = 0;
	virtual void StartPlayback() = 0;
	virtual void PrepareRecording() = 0;
};

#endif
