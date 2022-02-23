/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2021 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODOCUMENT_H
#define GODOCUMENT_H

#include <wx/string.h>

#include "document-base/GODocumentBase.h"
#include "midi/GOMidiCallback.h"
#include "midi/GOMidiListener.h"
#include "threading/GOMutex.h"

class GODefinitionFile;
class GOKeyReceiver;
class GOMidiEvent;
class GOMidiReceiverBase;
class GOMidiSender;
class GOOrgan;
class GOProgressDialog;
class GOResizable;
class GOSound;

class GODocument : public GODocumentBase, protected GOMidiCallback {
private:
  GOResizable *p_MainWindow;
  GOSound &m_sound;

  GOMutex m_lock;
  bool m_OrganFileReady;
  GODefinitionFile *m_organfile;

  GOMidiListener m_listener;
  bool m_modified;

  void OnMidiEvent(const GOMidiEvent &event);

  void SyncState();
  void CloseOrgan();

public:
  GODocument(GOResizable *pMainWindow, GOSound *sound);
  ~GODocument();

  bool IsModified();
  void Modify(bool modified);

  void ShowPanel(unsigned id);
  void ShowOrganDialog();
  void ShowMIDIEventDialog(
    void *element,
    wxString title,
    GOMidiReceiverBase *event,
    GOMidiSender *sender,
    GOKeyReceiver *key,
    GOMidiSender *division = NULL);
  void ShowMidiList();

  bool Load(GOProgressDialog *dlg, const GOOrgan &organ);
  bool Save();
  bool Export(const wxString &cmb);
  bool Import(GOProgressDialog *dlg, const GOOrgan &organ, const wxString &cmb);
  bool ImportCombination(const wxString &cmb);
  bool Revert(GOProgressDialog *dlg);
  bool UpdateCache(GOProgressDialog *dlg, bool compress);

  GODefinitionFile *GetOrganFile();
};

#endif
