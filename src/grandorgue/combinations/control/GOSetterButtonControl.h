/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTERBUTTONCONTROL_H
#define GOSETTERBUTTONCONTROL_H

#include "control/GOButtonControl.h"

class GOSetterButtonCallback;

class GOSetterButtonControl : public GOButtonControl {
protected:
  GOSetterButtonCallback *m_setter;

public:
  GOSetterButtonControl(
    GODefinitionFile *organfile,
    GOSetterButtonCallback *setter,
    bool Pushbutton);
  void Push(void);
  void Set(bool on);

  wxString GetMidiType();
};

#endif
