/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOMidiChannelMappingDialog.h"

#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

static const wxString WX_MAPPING_CHOICES[] = {
  _("Pedal first (Viscount style)"),
  _("Pedal last (old GrandOrgue style)"),
  _("Use the organ's MIDIInputNumber"),
};
static constexpr size_t MAPPING_CHOICE_CNT
  = sizeof(WX_MAPPING_CHOICES) / sizeof(wxString);

GOMidiChannelMappingDialog::GOMidiChannelMappingDialog(
  wxWindow *parent,
  GOConfig &config,
  bool isMidiInputNumberMappingUsable,
  GOConfig::MidiFileChannelMapping defaultMapping)
  : GOSimpleDialog(
    parent,
    "MidiChannelMapping",
    _("Choose MIDI channel mapping"),
    config.m_DialogSizes,
    wxEmptyString,
    0,
    wxOK | wxCANCEL) {
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText *label = new wxStaticText(
    this,
    wxID_ANY,
    _("This MIDI file does not contain GrandOrgue's own channel setup.\n"
      "Choose how to map MIDI channels to manuals:"));

  sizer->Add(label, 0, wxALL, 5);

  m_MappingChoice = new wxRadioBox(
    this,
    wxID_ANY,
    _("Channel mapping"),
    wxDefaultPosition,
    wxDefaultSize,
    MAPPING_CHOICE_CNT,
    WX_MAPPING_CHOICES,
    1,
    wxRA_SPECIFY_COLS);
  m_MappingChoice->SetSelection((int)defaultMapping);
  m_MappingChoice->Enable(
    GOConfig::MIDI_PLAY_CHANNELS_USE_INPUT_NUMBER,
    isMidiInputNumberMappingUsable);
  sizer->Add(m_MappingChoice, 0, wxALL | wxEXPAND, 5);

  LayoutWithInnerSizer(sizer);
}

GOConfig::MidiFileChannelMapping GOMidiChannelMappingDialog::
  GetSelectedMapping() const {
  return (GOConfig::MidiFileChannelMapping)m_MappingChoice->GetSelection();
}
