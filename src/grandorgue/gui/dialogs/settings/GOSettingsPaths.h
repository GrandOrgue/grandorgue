/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSPATHS_H
#define GOSETTINGSPATHS_H

#include <wx/panel.h>

class GOConfig;
class wxDirPickerCtrl;

class GOSettingsPaths : public wxPanel {
private:
  GOConfig &m_config;

  wxDirPickerCtrl *m_Samplesets;
  wxDirPickerCtrl *m_OrganPackages;
  wxDirPickerCtrl *m_OrganCache;
  wxDirPickerCtrl *m_OrganSettings;
  wxDirPickerCtrl *m_OrganCombinations;
  wxDirPickerCtrl *m_ExportImport;
  wxDirPickerCtrl *m_AudioRecorder;
  wxDirPickerCtrl *m_MidiRecorder;
  wxDirPickerCtrl *m_MidiPlayer;

public:
  GOSettingsPaths(GOConfig &settings, wxWindow *parent);

  virtual bool TransferDataFromWindow() override;
};

#endif
