/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUESETTERBUTTONCALLBACK_H
#define GORGUESETTERBUTTONCALLBACK_H

class GOrgueSetterButton;

class GOrgueSetterButtonCallback
{
public:
	virtual ~GOrgueSetterButtonCallback()
	{
	}

	virtual void SetterButtonChanged(GOrgueSetterButton* button) = 0;
};

#endif
