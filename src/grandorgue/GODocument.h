/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODOCUMENT_H
#define GODOCUMENT_H

#include <wx/string.h>

#include "document-base/GODocumentBase.h"
#include "midi/GOMidiCallback.h"
#include "midi/GOMidiListener.h"
#include "midi/dialog-creator/GOMidiDialogCreator.h"
#include "threading/GOMutex.h"

class GOOrganController;
class GOMidiShortcutReceiver;
class GOMidiEvent;
class GOMidiReceiverBase;
class GOMidiSender;
class GOOrgan;
class GOProgressDialog;
class GOResizable;
class GOSound;

class GODocument : public GODocumentBase,
                   protected GOMidiCallback,
                   public GOMidiDialogCreator {
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

  GOOrganController *GetOrganController() const { return m_OrganController; }
  bool IsModified() const;

  void ShowPanel(unsigned id);
  void ShowOrganSettingsDialog();
  void ShowMidiList();

  bool Save();
  bool Export(const wxString &cmb);
  bool LoadOrgan(
    GOProgressDialog *dlg,
    const GOOrgan &organ,
    const wxString &cmb,
    bool isGuiOnly);
  bool UpdateCache(GOProgressDialog *dlg, bool compress);

  void ShowMIDIEventDialog(
    void *element,
    const wxString &title,
    const wxString &dialogSelector,
    GOMidiReceiverBase *event,
    GOMidiSender *sender,
    GOMidiShortcutReceiver *key,
    GOMidiSender *division = nullptr) override;
};

#endif
