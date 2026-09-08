/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GODialogSizeSet.h"

#include "config/GOConfigReader.h"
#include "config/GOConfigWriter.h"

const wxString WX_EMPTY_STRING = wxEmptyString;

std::hash<std::wstring> GODialogSizeSet::SizeIdHash::instance;

const wxString WX_DIALOG_SIZES = wxT("DialogSizes");
const wxString WX_SAVED_COUNT = wxT("SavedCount");
const wxString WX_DIALOG_NAME_03D = wxT("DialogName%03d");
const wxString WX_DIALOG_SELECTOR_03D = wxT("DialogSelector%03d");

wxString dialog_group_name(
  const wxString &dialogName, const wxString &dialogSelector) {
  wxString groupName;

  groupName << WX_DIALOG_SIZES << "." << dialogName;
  if (!dialogSelector.IsEmpty())
    groupName << "." << dialogSelector;
  return groupName;
}

GOSizeKeeper &GODialogSizeSet::AssureSizeKeeperFor(
  const wxString &dialogName, const wxString &dialogSelector) {
  const SizeId sizeId(dialogName, dialogSelector);
  std::unique_ptr<GOSizeKeeper> &rSizeKeeper = m_SizeSet[sizeId];

  if (!rSizeKeeper) {
    rSizeKeeper = std::make_unique<GOSizeKeeper>();
    rSizeKeeper->SetGroup(dialog_group_name(dialogName, dialogSelector));
  }
  return *rSizeKeeper;
}

wxString format_dialog_name_key(unsigned index) {
  return wxString::Format(WX_DIALOG_NAME_03D, index + 1);
}

wxString format_dialog_selector_key(unsigned index) {
  return wxString::Format(WX_DIALOG_SELECTOR_03D, index + 1);
}

unsigned read_saved_count(GOConfigReader &cfg, GOSettingType settingType) {
  return cfg.ReadInteger(
    settingType, WX_DIALOG_SIZES, WX_SAVED_COUNT, 0, 998, false, 0);
}

bool GODialogSizeSet::isPresentInCfg(
  GOConfigReader &cfg, GOSettingType settingType) {
  return read_saved_count(cfg, settingType) > 0;
}

void GODialogSizeSet::Load(GOConfigReader &cfg, GOSettingType settingType) {
  /*
     The existing GOSizeKeeper objects are not removed but reused: every open
     GODialog holds a reference to its keeper for its whole lifetime, and Load()
     may run while some dialogs are still open (ex. when another organ is
     loaded). The keepers of the dialogs mentioned in cfg are overwritten below,
     the other ones keep their current values.
   */
  const unsigned savedCount = cfg.ReadInteger(
    settingType, WX_DIALOG_SIZES, WX_SAVED_COUNT, 0, 998, false, 0);

  for (unsigned i = 0; i < savedCount; i++) {
    const wxString dialogName = cfg.ReadString(
      settingType, WX_DIALOG_SIZES, format_dialog_name_key(i), false);
    const wxString dialogSelector = cfg.ReadString(
      settingType, WX_DIALOG_SIZES, format_dialog_selector_key(i), false);

    AssureSizeKeeperFor(dialogName, dialogSelector)
      .Load(cfg, dialog_group_name(dialogName, dialogSelector));
  }
}

void GODialogSizeSet::Save(GOConfigWriter &cfg) const {
  unsigned i = 0;

  cfg.WriteInteger(WX_DIALOG_SIZES, WX_SAVED_COUNT, m_SizeSet.size());
  for (auto &e : m_SizeSet) {
    cfg.WriteString(WX_DIALOG_SIZES, format_dialog_name_key(i), e.first.first);
    cfg.WriteString(
      WX_DIALOG_SIZES, format_dialog_selector_key(i), e.first.second);
    e.second->Save(cfg);
    i++;
  }
}
