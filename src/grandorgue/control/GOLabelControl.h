/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOLABELCONTROL_H
#define GOLABELCONTROL_H

#include <wx/string.h>

#include "midi/GOMidiConfigurator.h"
#include "midi/GOMidiSender.h"
#include "sound/GOSoundStateHandler.h"

#include "GOSaveableObject.h"

class GOConfigReader;
class GOConfigWriter;
class GOOrganController;

class GOLabelControl : private GOSaveableObject,
                       private GOSoundStateHandler,
                       public GOMidiConfigurator {
protected:
  wxString m_Name;
  wxString m_Content;
  wxString m_group;
  GOOrganController *m_OrganController;
  GOMidiSender m_sender;

  void Save(GOConfigWriter &cfg);

  void AbortPlayback();
  void PreparePlayback();
  void PrepareRecording();

public:
  GOLabelControl(GOOrganController *organController);
  virtual ~GOLabelControl();
  void Init(GOConfigReader &cfg, wxString group, wxString name);
  void Load(GOConfigReader &cfg, wxString group, wxString name);
  const wxString &GetName() const { return m_Name; }
  const wxString &GetContent();
  void SetContent(wxString name);

  const wxString &GetMidiTypeCode() const override;
  const wxString &GetMidiType() const override;
  const wxString &GetMidiName() const override { return GetName(); }
  GOMidiSender *GetMidiSender() override { return &m_sender; }

  wxString GetElementStatus();
  std::vector<wxString> GetElementActions();
  void TriggerElementActions(unsigned no);
};

#endif
