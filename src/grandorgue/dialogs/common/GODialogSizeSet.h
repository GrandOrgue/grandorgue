/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GODIALOGSIZESET_H
#define GODIALOGSIZESET_H

#include <tuple>
#include <unordered_map>

#include <wx/string.h>

#include "config/GOConfigReader.h"
#include "size/GOSizeKeeper.h"

/**
 * This class keeps positions and sizes of all dialogs. Every dialog is
 * identified by two wxString values:
 * 1. dialogName - mandatory. It is the same name as it is used for navigating
 * over help.
 * 2. dialogSelector - optional. It is used if the same dialog may be used
 * several times (ex. MidiEventDialog)
 */

class GOConfigWriter;

class GODialogSizeSet {
private:
  using SizeId = std::pair<wxString, wxString>;

  struct SizeIdHash {
    // Don't use std::hash<wxString> because it does not exist in wx 3.0
    static std::hash<std::wstring> instance;

    std::size_t operator()(const SizeId &k) const {
      return instance(k.first.ToStdWstring())
        ^ instance(k.second.ToStdWstring());
    }
  };

  struct SizeIdEqual {
    bool operator()(const SizeId &lhs, const SizeId &rhs) const {
      return lhs.first == rhs.first && lhs.second == rhs.second;
    }
  };

  std::unordered_map<SizeId, GOSizeKeeper *, SizeIdHash, SizeIdEqual> m_SizeSet;

public:
  ~GODialogSizeSet();

  bool IsEmpty() const { return m_SizeSet.empty(); }

  /**
   * Search a GOGUISizeKeeper instance for a dialog identified by dialogName and
   * pDialogSelector. If such element does not exist, it is created and
   * initialised with default.
   * @param dialogName
   * @param pDialogSelector
   * @return
   */
  GOSizeKeeper &AssureSizeKeeperFor(
    const wxString &dialogName, const wxString &dialogSelector = wxEmptyString);

  static bool isPresentInCfg(GOConfigReader &cfg, GOSettingType settingType);

  /**
   * Load the dialog size infos from the config file.
   * @param cfg
   */
  void Load(GOConfigReader &cfg, GOSettingType settingType);

  /**
   * Write all dialog size infos to the config file.
   * @param cfg
   */
  void Save(GOConfigWriter &cfg) const;

  /**
   * Copies all dialog infos from the src. The old content is cleared.
   * @param src
   * @return
   */
  GODialogSizeSet &operator=(const GODialogSizeSet &src);
};

#endif /* GODIALOGSIZESET_H */
