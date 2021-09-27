/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUELABEL_H
#define GORGUELABEL_H

#include "GOrgueMidiConfigurator.h"
#include "GOrgueMidiSender.h"
#include "GOrguePlaybackStateHandler.h"
#include "GOrgueSaveableObject.h"
#include <wx/string.h>

class GOrgueConfigReader;
class GOrgueConfigWriter;
class GrandOrgueFile;

class GOrgueLabel : private GOrgueSaveableObject, private GOrguePlaybackStateHandler, public GOrgueMidiConfigurator
{
protected:
	wxString m_Name;
	wxString m_Content;
	wxString m_group;
	GrandOrgueFile* m_organfile;
	GOrgueMidiSender m_sender;

	void Save(GOrgueConfigWriter& cfg);

	void AbortPlayback();
	void PreparePlayback();
	void StartPlayback();
	void PrepareRecording();

public:
	GOrgueLabel(GrandOrgueFile* organfile);
	virtual ~GOrgueLabel();
	void Init(GOrgueConfigReader& cfg, wxString group, wxString name);
	void Load(GOrgueConfigReader& cfg, wxString group, wxString name);
	const wxString& GetName();
	const wxString& GetContent();
	void SetContent(wxString name);

	wxString GetMidiType();
	wxString GetMidiName();
	void ShowConfigDialog();

	wxString GetElementStatus();
	std::vector<wxString> GetElementActions();
	void TriggerElementActions(unsigned no);
};

#endif
