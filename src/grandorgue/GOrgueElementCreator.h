/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEELEMENTCREATOR_H
#define GORGUEELEMENTCREATOR_H

#include "GOrgueSetterButtonCallback.h"
#include "ptrvector.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueButton;
class GOrgueEnclosure;
class GOrgueLabel;
class GOrgueSetterButton;
class GrandOrgueFile;

struct ElementListEntry {
	wxString name;
	int value;
	bool is_public;
	bool is_pushbutton;
};

class GOrgueElementCreator : private GOrgueSetterButtonCallback
{
private:
	void SetterButtonChanged(GOrgueSetterButton* button);

protected:
	ptr_vector<GOrgueSetterButton> m_button;

	virtual const struct ElementListEntry* GetButtonList() = 0;
	virtual void ButtonChanged(int id) = 0;
	void CreateButtons(GrandOrgueFile* organfile);

public:
	GOrgueElementCreator();
	virtual ~GOrgueElementCreator();

	virtual void Load(GOrgueConfigReader& cfg) = 0;

	virtual GOrgueEnclosure* GetEnclosure(const wxString& name, bool is_panel) = 0;
	virtual GOrgueLabel* GetLabel(const wxString& name, bool is_panel) = 0;
	virtual GOrgueButton* GetButton(const wxString& name, bool is_panel);
};

#endif
