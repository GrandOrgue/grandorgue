/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSETTINGSOPTIONS_H
#define GOSETTINGSOPTIONS_H

#include <wx/panel.h>

enum class GOInitialLoadType;
template <class T> class GOChoice;
class GOConfig;
class wxCheckBox;
class wxChoice;
class wxDirPickerCtrl;
class wxSpinCtrl;

class GOSettingsOptions : public wxPanel {
  enum {
    ID_WAVE_FORMAT = 200,
    ID_CONCURRENCY,
    ID_RELEASE_CONCURRENCY,
    ID_LOAD_CONCURRENCY,
    ID_LOSSLESS_COMPRESSION,
    ID_MANAGE_POLYPHONY,
    ID_COMPRESS_CACHE,
    ID_MANAGE_CACHE,
    ID_SCALE_RELEASE,
    ID_LOAD_LAST_FILE,
    ID_RANDOMIZE,
    ID_BITS_PER_SAMPLE,
    ID_LOOP_LOAD,
    ID_ATTACK_LOAD,
    ID_RELEASE_LOAD,
    ID_CHANNELS,
    ID_INTERPOLATION,
    ID_MEMORY_LIMIT,
    ID_ODF_CHECK,
    ID_RECORD_DOWNMIX,
    ID_VOLUME,
    ID_LANGUAGE,
    ID_METRONOME_MEASURE,
    ID_METRONOME_BPM,
  };

private:
  GOConfig &m_config;
  wxChoice *m_Concurrency;
  wxChoice *m_ReleaseConcurrency;
  wxChoice *m_LoadConcurrency;
  wxChoice *m_WaveFormat;
  wxCheckBox *m_LosslessCompression;
  wxCheckBox *m_Limit;
  wxCheckBox *m_CompressCache;
  wxCheckBox *m_ManageCache;
  GOChoice<GOInitialLoadType> *m_LoadLastFile;
  wxCheckBox *m_Scale;
  wxCheckBox *m_Random;
  wxCheckBox *m_ODFCheck;
  wxCheckBox *m_ODFHw1Check;
  wxCheckBox *m_RecordDownmix;
  wxSpinCtrl *m_Volume;
  wxChoice *m_BitsPerSample;
  wxChoice *m_LoopLoad;
  wxChoice *m_AttackLoad;
  wxChoice *m_ReleaseLoad;
  wxChoice *m_Channels;
  wxChoice *m_Interpolation;
  wxSpinCtrl *m_MemoryLimit;
  wxChoice *m_Language;
  wxSpinCtrl *m_MetronomeMeasure;
  wxSpinCtrl *m_MetronomeBPM;
  wxCheckBox *m_CheckForUpdatesAtStartup;

  wxString m_OldLanguageCode;
  unsigned m_OldChannels;
  bool m_OldLosslessCompression;
  unsigned m_OldBitsPerSample;
  unsigned m_OldLoopLoad;
  unsigned m_OldAttackLoad;
  unsigned m_OldReleaseLoad;

public:
  GOSettingsOptions(GOConfig &settings, wxWindow *parent);

  bool NeedReload();
  bool NeedRestart();

  virtual bool TransferDataFromWindow() override;
};

#endif
