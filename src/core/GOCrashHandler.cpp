/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifdef __WIN32__
/*
 * Enables the POSIX names (open, write, close, getpid) of the CRT functions:
 * MinGW declares them deprecated otherwise. It must be defined before any
 * system header is included.
 */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "GOCrashHandler.h"

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <string>

#ifdef __WIN32__
#include <io.h>
#include <process.h>
#include <windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif

#include <sys/stat.h>

#include <wx/filename.h>

#if defined(GO_HAS_CPPTRACE)
#include <cpptrace/cpptrace.hpp>
#endif

#include "go_path.h"

/*
 * The report is written with plain C and POSIX calls (open, write, close)
 * instead of the C++ streams and of wxFile, wxLogError and so on because a
 * report may be written from a signal handler. Everything called from there
 * must be async-signal-safe: it may neither allocate memory, nor take locks,
 * nor touch the locale, because the interrupted code may hold the very same
 * heap or lock. std::ofstream, printf() and wxString do all of that, while
 * write() is listed as async-signal-safe. The same reason rules out
 * std::format and wxDateTime while formatting: only std::to_chars, that
 * neither allocates nor uses the locale, and preformatted constants are used.
 *
 * The stack trace is taken with cpptrace rather than with std::stacktrace
 * because the latter requires C++23 while the project is built as C++20.
 *
 * On Unix the handlers are installed with sigaction() rather than with
 * std::signal() or with wxHandleFatalExceptions() and
 * wxApp::OnFatalException():
 * - std::signal() provides neither SA_ONSTACK, without which a stack overflow
 *   cannot be reported at all, nor SA_RESETHAND;
 * - the wx mechanism is compiled into wxWidgets only when
 *   wxUSE_ON_FATAL_EXCEPTION is set and, on Windows, only for Visual C++. Our
 *   MinGW wx build does not even export wxHandleFatalExceptions, so Windows
 *   needs SetUnhandledExceptionFilter anyway;
 * - the wx mechanism reports through wxTheApp, so every application
 *   (GrandOrgue, GrandOrgueTool, GrandOrguePerfTest) would have to override
 *   OnFatalException, while this class is usable as is by any of them.
 * Handling the signals ourselves keeps the behaviour identical everywhere.
 *
 * All this applies to the crash time only: ensureInstalled() runs in a healthy
 * process, so it uses wxString and the C++ library freely.
 */

// The maximum length of the report file path, including the terminating zero
static constexpr unsigned MAX_REPORT_PATH_LEN = 1024;
// The minimal length of a buffer for format_hex()
static constexpr unsigned HEX_TEXT_LEN = 24;
// The file descriptor of stderr
static constexpr int STDERR_FD = 2;

static bool is_installed = false;
/*
 * Guards the part of the report that allocates memory: if resolving the stack
 * trace crashes itself then the signal-safe part has already been written.
 */
static bool is_resolving_trace = false;
// The report file descriptor. It is valid only while a report is being written
static int report_fd = -1;

/*
 * The report file path is precomputed when the handler is installed: nothing
 * may be allocated nor converted at the crash time. On Windows it is stored as
 * wchar_t because the path may contain characters that are not representable
 * in the current ANSI codepage.
 */
#ifdef __WIN32__
static wchar_t report_path[MAX_REPORT_PATH_LEN] = L"";
#else
static char report_path[MAX_REPORT_PATH_LEN] = "";
#endif

/**
 * Writes the text to the file descriptor
 */
static void write_to_fd(int fd, const char *pText) {
  unsigned nRest = (unsigned)strlen(pText);

  while (nRest > 0) {
    const int nWritten = (int)write(fd, pText, nRest);

    if (nWritten > 0) {
      pText += nWritten;
      nRest -= nWritten;
    } else // the descriptor is broken: give up rather than loop forever
      nRest = 0;
  }
}

/**
 * Writes the text to both stderr and the report file
 */
static void write_text(const char *pText) {
  write_to_fd(STDERR_FD, pText);
  if (report_fd >= 0)
    write_to_fd(report_fd, pText);
}

#if defined(GO_HAS_CPPTRACE)

/**
 * Writes the stack trace of the calling thread.
 *
 * Taking and resolving the trace allocates memory, so it is the only part of
 * the report that is not signal-safe: it may deadlock or crash if the fatal
 * signal has interrupted the allocator itself. cpptrace could unwind in a
 * signal-safe way (cpptrace::safe_generate_raw_trace), but only when it is
 * built with libunwind, which none of our builds is. Therefore the reason of
 * the crash is written before this call: the report is not empty even when
 * this function does not return.
 */
static void write_stack_trace() {
  if (!is_resolving_trace) {
    is_resolving_trace = true;
    try {
      const std::string stackTrace = cpptrace::generate_trace(1).to_string();

      write_text(stackTrace.c_str());
      write_text("\n");
    } catch (...) {
      write_text("Resolving the stack trace failed\n");
    }
    is_resolving_trace = false;
  }
}

#else

static void write_stack_trace() {
  write_text("Built without cpptrace: no stack trace is available\n");
}

#endif /* GO_HAS_CPPTRACE */

static int open_report_file() {
  int fd = -1;

  if (report_path[0])
#ifdef __WIN32__
    // there is no POSIX call taking a wide character path
    fd
      = _wopen(report_path, O_WRONLY | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE);
#else
    fd = open(report_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
#endif
  return fd;
}

void GOCrashHandler::reportFatalError(const char *pReason) {
  report_fd = open_report_file();
  write_text("\n*** GrandOrgue fatal error: ");
  write_text(pReason);
  write_text(" ***\n");
  write_stack_trace();
  write_text("*** End of the fatal error report ***\n");
  if (report_fd >= 0) {
    close(report_fd);
    report_fd = -1;
  }
}

#ifdef __WIN32__

/**
 * Formats the value as a 0x-prefixed hexadecimal number
 * @param value the value to format
 * @param outText the buffer to write to. Must be at least HEX_TEXT_LEN long
 */
static void format_hex(uint64_t value, char *outText) {
  const std::to_chars_result result
    = std::to_chars(outText + 2, outText + HEX_TEXT_LEN - 1, value, 16);

  outText[0] = '0';
  outText[1] = 'x';
  *result.ptr = 0;
}

static LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS *pExcInfo) {
  char reason[HEX_TEXT_LEN + 32] = "unhandled exception ";

  format_hex(pExcInfo->ExceptionRecord->ExceptionCode, reason + strlen(reason));
  GOCrashHandler::reportFatalError(reason);
  /*
   * EXCEPTION_EXECUTE_HANDLER terminates the process at once. Otherwise the
   * Windows Error Reporting service would be launched and the process would
   * stay alive waiting for it, looking hung rather than crashed.
   */
  return EXCEPTION_EXECUTE_HANDLER;
}

static void install_handler() {
  SetUnhandledExceptionFilter(unhandled_exception_filter);
}

#else

// The stack the signal handler runs on: the process stack may be exhausted
static char alt_stack_buffer[128 * 1024];

static void fatal_signal_handler(int signalN) {
  const char *pReason;

  switch (signalN) {
  case SIGSEGV:
    pReason = "SIGSEGV (invalid memory access)";
    break;
  case SIGBUS:
    pReason = "SIGBUS (bus error)";
    break;
  case SIGFPE:
    pReason = "SIGFPE (arithmetic error)";
    break;
  case SIGILL:
    pReason = "SIGILL (illegal instruction)";
    break;
  case SIGABRT:
    pReason = "SIGABRT (aborted)";
    break;
  default:
    pReason = "fatal signal";
    break;
  }
  GOCrashHandler::reportFatalError(pReason);
  /*
   * SA_RESETHAND has restored the default handler, so re-raising kills the
   * process with the original signal and a core dump is still produced.
   */
  raise(signalN);
}

static void install_handler() {
  static const int FATAL_SIGNALS[] = {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT};

  stack_t altStack;
  struct sigaction action;

  altStack.ss_sp = alt_stack_buffer;
  altStack.ss_size = sizeof(alt_stack_buffer);
  altStack.ss_flags = 0;
  sigaltstack(&altStack, nullptr);

  memset(&action, 0, sizeof(action));
  action.sa_handler = fatal_signal_handler;
  action.sa_flags = SA_ONSTACK | SA_RESETHAND;
  sigemptyset(&action.sa_mask);
  for (int signalN : FATAL_SIGNALS)
    sigaction(signalN, &action, nullptr);
}

#endif /* __WIN32__ */

void GOCrashHandler::ensureInstalled(const wxString &reportDirPath) {
  if (!is_installed) {
    is_installed = true;
    if (go_create_directory(reportDirPath)) {
      const std::time_t now = std::time(nullptr);

      char timeText[32];
      char fileName[64];

      std::strftime(
        timeText, sizeof(timeText), "%Y%m%d-%H%M%S", std::localtime(&now));
      std::snprintf(
        fileName,
        sizeof(fileName),
        "crash-%s-%u.txt",
        timeText,
        (unsigned)getpid());

      const wxString filePath
        = reportDirPath + wxFileName::GetPathSeparator() + fileName;

#ifdef __WIN32__
      wcsncpy(report_path, filePath.wc_str(), MAX_REPORT_PATH_LEN - 1);
#else
      strncpy(report_path, filePath.utf8_str().data(), MAX_REPORT_PATH_LEN - 1);
#endif
    }
    install_handler();
  }
}
