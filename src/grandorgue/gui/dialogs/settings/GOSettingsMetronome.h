/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSMETRONOME_H
#define GOSETTINGSMETRONOME_H

#include "gui/dialogs/common/GODialogTab.h"

class wxFileDirPickerEvent;
class wxRadioBox;
class wxSpinCtrl;
class wxString;

class GOConfig;
class GOFilePickerCtrl;

class GOSettingsMetronome : public GODialogTab {
private:
  GOConfig &r_config;

  wxSpinCtrl *m_measure;
  wxSpinCtrl *m_bpm;
  wxRadioBox *m_SoundType;
  GOFilePickerCtrl *m_FirstBeatPath;
  GOFilePickerCtrl *m_BeatPath;

  unsigned m_OldSoundType;
  wxString m_OldFirstBeatPath;
  wxString m_OldBeatPath;

public:
  GOSettingsMetronome(
    GOConfig &config,
    GOTabbedDialog *pDlg,
    const wxString &name,
    const wxString &label);

private:
  void OnSoundTypeChanged(unsigned soundType);
  void OnSoundTypeChanged(wxCommandEvent &event);
  void OnSampleFileChanged(wxFileDirPickerEvent &event);
  
public:
  bool Validate() override;

  bool TransferDataToWindow() override;
  bool TransferDataFromWindow() override;

  bool NeedReload();

  DECLARE_EVENT_TABLE()
};

#endif /* GOSETTINGSMETRONOME_H */
