/*
 * GrandOrgue - a free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GOAPP_H
#define GOAPP_H

#include <memory>

#include <wx/app.h>

class GOConfig;
class GOFrame;
class GOLog;
class GOSoundSystem;

class GOApp : public wxApp {
private:
  /**
   * A temporary logging class.
   * It logs all Warning and Error log messages to stderr, all other messages to
   * stdout. It alse displays all Error messages to a modal message box.
   * It is used only before initialising the GOLog instance, including during
   * reading the GrandOrgueConfig
   */
  class TemporaryLog : public wxLog {
  protected:
    void DoLogTextAtLevel(wxLogLevel level, const wxString &msg) override;
  };

  // A temporary logging instance
  std::unique_ptr<TemporaryLog> mp_TemporaryLog
    = std::make_unique<TemporaryLog>();
  bool m_IsToRestartAfterExit = false;

#ifdef __WXMAC__
  virtual void MacOpenFile(const wxString &fileName) override;
#endif
  void OnInitCmdLine(wxCmdLineParser &parser) override;
  bool OnCmdLineParsed(wxCmdLineParser &parser) override;
  bool OnInit() override;
  int OnRun() override;
  int OnExit() override;
  virtual void CleanUp() override;

protected:
  GOFrame *p_frame = nullptr;
  wxLocale m_locale;
  std::unique_ptr<GOConfig> mp_config;
  std::unique_ptr<GOSoundSystem> mp_SoundSystem;
  std::unique_ptr<GOLog> mp_log;
  wxString m_FileName;
  std::string m_InstanceName;
  std::string m_ConfigFilePath;
  bool m_IsGuiOnly = false;

public:
  ~GOApp();

  bool IsToRestartAfterExit() const { return m_IsToRestartAfterExit; }
  void SetToRestartAfterExit() { m_IsToRestartAfterExit = true; }
};

DECLARE_APP(GOApp)

#endif
