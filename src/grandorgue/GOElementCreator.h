/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOELEMENTCREATOR_H
#define GOELEMENTCREATOR_H

#include <wx/string.h>

#include "GOSetterButtonCallback.h"
#include "ptrvector.h"

class GOConfigReader;
class GOButton;
class GOEnclosure;
class GOLabel;
class GOSetterButton;
class GODefinitionFile;

struct ElementListEntry {
  wxString name;
  int value;
  bool is_public;
  bool is_pushbutton;
};

class GOElementCreator : private GOSetterButtonCallback {
private:
  void SetterButtonChanged(GOSetterButton *button);

protected:
  ptr_vector<GOSetterButton> m_button;

  virtual const struct ElementListEntry *GetButtonList() = 0;
  virtual void ButtonChanged(int id) = 0;
  void CreateButtons(GODefinitionFile *organfile);

public:
  GOElementCreator();
  virtual ~GOElementCreator();

  virtual void Load(GOConfigReader &cfg) = 0;

  virtual GOEnclosure *GetEnclosure(const wxString &name, bool is_panel) = 0;
  virtual GOLabel *GetLabel(const wxString &name, bool is_panel) = 0;
  virtual GOButton *GetButton(const wxString &name, bool is_panel);
};

#endif
