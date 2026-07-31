/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCRASHHANDLER_H
#define GOCRASHHANDLER_H

#include <wx/string.h>

/**
 * Reporting of fatal errors that cannot be caught as C++ exceptions: access
 * violations, SIGSEGV, SIGBUS and the like.
 *
 * Without such a handler a fatal error leaves no diagnostics at all in a
 * release build. On Windows it is even worse: the process stays alive forever
 * inside the Windows Error Reporting machinery, so the application looks hung
 * rather than crashed.
 *
 * The handler writes a stack trace to stderr and to a file in the report
 * directory and then lets the process die immediately.
 */
class GOCrashHandler {
public:
  /**
   * Installs the platform fatal error handler. Idempotent: repeated calls do
   * nothing.
   *
   * Creates the report directory and precomputes the report file path here
   * because neither memory allocation nor wxString may be used while handling
   * a fatal error.
   * @param reportDirPath the directory the report files are written to
   */
  static void ensureInstalled(const wxString &reportDirPath);

  /**
   * Writes a fatal error report: the reason and the stack trace of the
   * calling thread. May be called from a signal handler: it allocates nothing
   * until the signal-safe part of the report has been written.
   * @param pReason a short description of what has happened
   */
  static void reportFatalError(const char *pReason);
};

#endif /* GOCRASHHANDLER_H */
