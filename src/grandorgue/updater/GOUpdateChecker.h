/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOUPDATECHECKER_H
#define GOUPDATECHECKER_H

#include "threading/GOThread.h"
#include <stdexcept>
#include <string>
#include <wx/wx.h>

struct GOUpdateChecker {
  struct ReleaseMetadata {
    wxString version;
    wxString changelog;
  };

  struct Result {
    ReleaseMetadata latestRelease;
    bool updateAvailable;
  };

  class CompletionEvent : public wxEvent {
  private:
    Result m_result;

  public:
    CompletionEvent(wxEventType eventType, Result result)
      : wxEvent(0, eventType), m_result(std::move(result)) {}

    CompletionEvent(const CompletionEvent &event) {
      this->m_result = event.m_result;
    }

    const Result &GetResult() const { return m_result; }

    virtual wxEvent *Clone() const override {
      return new CompletionEvent(*this);
    }
  };

  // The spawned thread fires UpdateCheckerCompletedEvent to
  // completionEventHandler on success. Nothing is done on failure except
  // logging some messages.
  static std::unique_ptr<GOThread> StartThread(
    wxEvtHandler *completionEventHandler);
};

/* Boilerplate code for custom event types in wxWidgets */
wxDECLARE_EVENT(UPDATE_CHECKING_COMPLETION, GOUpdateChecker::CompletionEvent);
typedef void (wxEvtHandler::*UpdateCheckingHandlerFunction)(
  GOUpdateChecker::CompletionEvent &);
#define UpdateCheckingEventHandler(func)                                       \
  wxEVENT_HANDLER_CAST(UpdateCheckingHandlerFunction, func)
#define EVT_UPDATE_CHECKING_COMPLETION(func)                                   \
  wx__DECLARE_EVT1(                                                            \
    UPDATE_CHECKING_COMPLETION, wxID_ANY, UpdateCheckingEventHandler(func))

#endif
