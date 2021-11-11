/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOGUISETTERDISPLAYMETRICS_H
#define GOGUISETTERDISPLAYMETRICS_H

#include "GOGUIDisplayMetrics.h"

typedef enum { GOGUI_SETTER_SETTER, GOGUI_SETTER_GENERALS, GOGUI_SETTER_CRESCENDO, GOGUI_SETTER_DIVISIONALS, GOGUI_SETTER_COUPLER, GOGUI_SETTER_MASTER, GOGUI_METRONOME, GOGUI_SETTER_FLOATING, GOGUI_RECORDER } GOGUISetterType;

class GOConfigReader;
class GODefinitionFile;

class GOGUISetterDisplayMetrics : public GOGUIDisplayMetrics
{
public:
	GOGUISetterDisplayMetrics(GOConfigReader& ini, GODefinitionFile* organfile, GOGUISetterType type);
};

#endif
