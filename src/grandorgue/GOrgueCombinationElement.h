/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUECOMBINATIONELEMENT_H
#define GORGUECOMBINATIONELEMENT_H

class GOrgueCombinationElement
{
public:
	virtual ~GOrgueCombinationElement()
	{
	}

	virtual bool GetCombinationState() const = 0;
	virtual void SetCombination(bool on) = 0;
};

#endif
