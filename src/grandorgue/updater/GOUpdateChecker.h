/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOUPDATECHECKER_H
#define GOUPDATECHECKER_H

#include <stdexcept>
#include <string>
#include <thread>
#include <wx/wx.h>

struct GOReleaseMetadata {
  wxString version;
  wxString changelog;
};

struct GOUpdateCheckerResult {
  GOReleaseMetadata latestRelease;
  bool updateAvailable;
};

class UpdateCheckerCompletedEvent;
wxDECLARE_EVENT(UPDATE_CHECKER_COMPLETED, UpdateCheckerCompletedEvent);

class UpdateCheckerCompletedEvent : public wxEvent {
private:
  GOUpdateCheckerResult m_result;

public:
  UpdateCheckerCompletedEvent(GOUpdateCheckerResult result)
    : wxEvent(0, UPDATE_CHECKER_COMPLETED), m_result(std::move(result)) {}

  UpdateCheckerCompletedEvent(const UpdateCheckerCompletedEvent &event) {
    this->m_result = event.m_result;
  }

  const GOUpdateCheckerResult &Result() const { return m_result; }

  virtual wxEvent *Clone() const override {
    return new UpdateCheckerCompletedEvent(*this);
  }
};

typedef void (wxEvtHandler::*UpdateCheckerHandlerFunction)(
  UpdateCheckerCompletedEvent &);
#define UpdateCheckerEventHandler(func)                                        \
  wxEVENT_HANDLER_CAST(UpdateCheckerHandlerFunction, func)
#define EVT_UPDATE_CHECKER_COMPLETED(func)                                     \
  wx__DECLARE_EVT1(                                                            \
    UPDATE_CHECKER_COMPLETED, wxID_ANY, UpdateCheckerEventHandler(func))

// The spawned thread fires UpdateCheckerCompletedEvent to
// completionEventHandler on success. Nothing is done on failure except logging
// some messages.
std::thread start_update_checker_thread(wxEvtHandler *completionEventHandler);

#endif
