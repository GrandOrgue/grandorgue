/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSCOPE_H
#define GOTESTSCOPE_H

#include <chrono>
#include <string>

/**
 * RAII progress logger for the test framework. Prints an indented,
 * immediately flushed line when a test class or a test starts, and another
 * when it finishes (with the elapsed time). Nesting depth is tracked
 * automatically, so a CI log always shows exactly which test class and
 * which test were in flight if the process hangs.
 */
class GOTestScope {
public:
  enum Kind { TEST_CLASS, TEST };

private:
  static int s_Depth;

  const Kind m_Kind;
  const std::string m_Name;
  const std::chrono::steady_clock::time_point m_StartTime;

public:
  GOTestScope(Kind kind, const std::string &name);
  ~GOTestScope();
};

/**
 * Runs callExpr as a Test, logged via GOTestScope. The test name is derived
 * from the call expression itself. Called without a trailing semicolon, like
 * the other declaration-style macros in this codebase.
 */
#define GO_RUN_TEST(callExpr)                                                  \
  {                                                                            \
    GOTestScope goTestScope_(GOTestScope::TEST, #callExpr);                    \
    callExpr;                                                                  \
  }

#endif /* GOTESTSCOPE_H */
