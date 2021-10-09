/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GORGUEDOCUMENT_H
#define GORGUEDOCUMENT_H

#include "GOrgueDocumentBase.h"
#include "GOrgueMidiCallback.h"
#include "GOrgueMidiListener.h"
#include "threading/GOMutex.h"
#include <wx/string.h>

class GOrgueKeyReceiver;
class GOrgueMidiEvent;
class GOrgueMidiReceiverBase;
class GOrgueMidiSender;
class GOrgueOrgan;
class GOrgueProgressDialog;
class GOrgueSound;
class GrandOrgueFile;

class GOrgueDocument : public GOrgueDocumentBase, protected GOrgueMidiCallback
{
private:
	GOMutex m_lock;
	bool m_OrganFileReady;
	GrandOrgueFile* m_organfile;
	GOrgueSound& m_sound;
	GOrgueMidiListener m_listener;
	bool m_modified;

	void OnMidiEvent(const GOrgueMidiEvent& event);

	void SyncState();
	void CloseOrgan();

public:
	GOrgueDocument(GOrgueSound* sound);
	~GOrgueDocument();

	bool IsModified();
	void Modify(bool modified);

	void ShowPanel(unsigned id);
	void ShowOrganDialog();
	void ShowMIDIEventDialog(void* element, wxString title, GOrgueMidiReceiverBase* event, GOrgueMidiSender* sender, GOrgueKeyReceiver* key, GOrgueMidiSender* division = NULL);
	void ShowMidiList();

	bool Load(GOrgueProgressDialog* dlg, const GOrgueOrgan& organ);
	bool Save();
	bool Export(const wxString& cmb);
	bool Import(GOrgueProgressDialog* dlg, const GOrgueOrgan& organ, const wxString& cmb);
	bool ImportCombination(const wxString& cmb);
	bool Revert(GOrgueProgressDialog* dlg);
	bool UpdateCache(GOrgueProgressDialog* dlg, bool compress);

	GrandOrgueFile* GetOrganFile();
};

#endif
