#ifndef GOSETTINGSREASON_H
#define GOSETTINGSREASON_H

#include <wx/string.h>

#include <vector>

struct GOSettingsReason {
  const wxString m_ReasonMessage;
  const size_t m_SettingsPageNum;

  GOSettingsReason(wxString reasonMessage, size_t settingsPageNum)
      : m_ReasonMessage(reasonMessage), m_SettingsPageNum(settingsPageNum) {}
};

typedef std::vector<GOSettingsReason> SettingsReasons;

#endif /* GOSETTINGSREASON_H */
