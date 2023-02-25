/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
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

class GOOrganController;
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
  GOOrganController *m_OrganController;

  GOMidiListener m_listener;

  void OnMidiEvent(const GOMidiEvent &event);

  void SyncState();
  void CloseOrgan();

public:
  GODocument(GOResizable *pMainWindow, GOSound *sound);
  ~GODocument();

  bool IsModified() const;

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

  bool Save();
  bool Export(const wxString &cmb);
  bool LoadOrgan(
    GOProgressDialog *dlg, const GOOrgan &organ, const wxString &cmb);
  bool ImportCombination(const wxString &cmb);
  bool UpdateCache(GOProgressDialog *dlg, bool compress);

  GOOrganController *GetOrganController() const { return m_OrganController; }
};

#endif
