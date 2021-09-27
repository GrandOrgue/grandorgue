/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESTATISTICCALLBACK_H
#define GORGUESTATISTICCALLBACK_H

class GOrgueSampleStatistic;

class GOrgueStatisticCallback
{
public:
	virtual ~GOrgueStatisticCallback()
	{
	}

	virtual GOrgueSampleStatistic GetStatistic() = 0;
};

#endif
