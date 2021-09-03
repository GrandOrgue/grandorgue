#ifndef SETTINGSREASON_H
#define SETTINGSREASON_H

#include <vector>
#include <wx/string.h>

struct SettingsReason
{
  const wxString m_ReasonMessage;
  const size_t m_SettingsPageNum;

  SettingsReason(wxString reasonMessage, size_t settingsPageNum):
    m_ReasonMessage(reasonMessage), m_SettingsPageNum(settingsPageNum)
  { }
};

typedef std::vector<SettingsReason> SettingsReasons;

#endif /* SETTINGSREASON_H */

