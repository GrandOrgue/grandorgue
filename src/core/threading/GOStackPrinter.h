/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOSTACKPRINTER_H
#define GOSTACKPRINTER_H

#include <wx/log.h>
#include <wx/stackwalk.h>

#if wxUSE_STACKWALKER
class GOStackPrinter : public wxStackWalker {
  void *m_ptr;

  GOStackPrinter(void *p) { m_ptr = p; }

  void OnStackFrame(const wxStackFrame &frame) {
    wxLogWarning(
      wxT("%p: [%2d] %s(%p)\t%s:%d"),
      m_ptr,
      frame.GetLevel(),
      frame.GetName().c_str(),
      frame.GetAddress(),
      frame.GetFileName().c_str(),
      frame.GetLine());
  }

public:
  static void printStack(void *p) {
    GOStackPrinter print(p);
    print.Walk();
  }
};
#else
class GOStackPrinter {
public:
  static void printStack(void *p) {}
};
#endif

#endif /* GOSTACKPRINTER_H */
