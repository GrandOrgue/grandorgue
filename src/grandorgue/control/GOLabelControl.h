/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLABELCONTROL_H
#define GOLABELCONTROL_H

#include <wx/string.h>

#include "GOPlaybackStateHandler.h"
#include "GOSaveableObject.h"
#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiSender.h"

class GOConfigReader;
class GOConfigWriter;
class GODefinitionFile;

class GOLabelControl : private GOSaveableObject,
                       private GOPlaybackStateHandler,
                       public GOMidiConfigurator {
protected:
  wxString m_Name;
  wxString m_Content;
  wxString m_group;
  GODefinitionFile *m_organfile;
  GOMidiSender m_sender;

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();
  void StartPlayback();
  void PrepareRecording();

public:
  GOLabelControl(GODefinitionFile *organfile);
  virtual ~GOLabelControl();
  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group, wxString name);
  const wxString &GetName();
  const wxString &GetContent();
  void SetContent(wxString name);

  wxString GetMidiType();
  wxString GetMidiName();
  void ShowConfigDialog();

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif
