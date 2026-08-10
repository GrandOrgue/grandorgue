/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDICHANNELMAPPINGDIALOG_H
#define GOMIDICHANNELMAPPINGDIALOG_H

#include "common/GOSimpleDialog.h"
#include "config/GOConfig.h"

class wxRadioBox;

/**
 * A modal dialog asking the user which GOConfig::MidiFileChannelMapping
 * scheme to use for a MIDI file lacking GrandOrgue's own setup header.
 */
class GOMidiChannelMappingDialog : public GOSimpleDialog {
private:
  wxRadioBox *m_MappingChoice;

public:
  /**
   * @param parent the parent window
   * @param config used for the dialog's persisted size/position
   * @param isMidiInputNumberMappingUsable whether
   *   MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER is offered as an enabled choice
   * @param defaultMapping the scheme pre-selected when the dialog opens
   */
  GOMidiChannelMappingDialog(
    wxWindow *parent,
    GOConfig &config,
    bool isMidiInputNumberMappingUsable,
    GOConfig::MidiFileChannelMapping defaultMapping);

  GOConfig::MidiFileChannelMapping GetSelectedMapping() const;
};

#endif /* GOMIDICHANNELMAPPINGDIALOG_H */
