/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSREVERB_H
#define GOSETTINGSREVERB_H

#include <wx/panel.h>

class GOConfig;
class wxCheckBox;
class wxChoice;
class wxFileDirPickerEvent;
class wxFilePickerCtrl;
class wxSpinButton;
class wxSpinCtrl;
class wxSpinEvent;
class wxStaticText;
class wxTextCtrl;

class GOSettingsReverb : public wxPanel {
  enum {
    ID_ENABLED = 200,
    ID_DIRECT,
    ID_FILE,
    ID_CHANNEL,
    ID_START_OFFSET,
    ID_LENGTH,
    ID_DELAY,
    ID_GAIN,
    ID_GAIN_SPIN,
  };

private:
  GOConfig &m_config;
  wxCheckBox *m_Enabled;
  wxCheckBox *m_Direct;
  wxFilePickerCtrl *m_File;
  wxChoice *m_Channel;
  wxSpinCtrl *m_StartOffset;
  wxSpinCtrl *m_Length;
  wxSpinCtrl *m_Delay;
  wxTextCtrl *m_Gain;
  wxSpinButton *m_GainSpin;

  void OnEnabled(wxCommandEvent &event);
  void OnFileChanged(wxFileDirPickerEvent &e);
  void OnGainSpinChanged(wxSpinEvent &e);
  void OnGainChanged(wxCommandEvent &e);

  void UpdateFile();
  void UpdateLimits();
  void UpdateEnabled();

  bool Validate();

public:
  GOSettingsReverb(GOConfig &settings, wxWindow *parent);

  virtual bool TransferDataFromWindow() override;

  DECLARE_EVENT_TABLE()
};

#endif
