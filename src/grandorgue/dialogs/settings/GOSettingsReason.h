#ifndef GOSETTINGSREASON_H
#define GOSETTINGSREASON_H

#include <wx/string.h>

#include <vector>

struct GOSettingsReason {
  const wxString m_ReasonMessage;
  const wxString m_SettingsPageName;

  GOSettingsReason(
    const wxString &reasonMessage, const wxString &settingsPageName)
    : m_ReasonMessage(reasonMessage), m_SettingsPageName(settingsPageName) {}
};

typedef std::vector<GOSettingsReason> SettingsReasons;

#endif /* GOSETTINGSREASON_H */
