/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSCOPE_H
#define GOTESTSCOPE_H

#include <chrono>
#include <string>
#include <vector>

/**
 * RAII progress logger for the test framework. Prints an indented,
 * immediately flushed line when a test class or a test starts, and another
 * when it finishes (with the elapsed time). Nesting depth is tracked
 * automatically, so a CI log always shows exactly which test class and
 * which test were in flight if the process hangs.
 *
 * It also records every finished scope with its elapsed time, so a timing
 * summary can be printed at the end.
 */
class GOTestScope {
public:
  enum Kind { TEST_CLASS, TEST };

  /**
   * A finished scope with its elapsed time
   */
  struct Record {
    Kind m_kind;
    std::string m_path; // "Class" or "Class/Test"
    long m_ElapsedMs;
  };

private:
  static int s_Depth;
  static std::string s_CurrentClass;
  static std::vector<Record> s_records;
  static bool s_IsProbing;
  static unsigned s_NCalls;
  static unsigned s_NSelectedInClass;

  const Kind m_Kind;
  const std::string m_Name;
  const std::string m_path; // "Class" or "Class/Test"
  const std::chrono::steady_clock::time_point m_StartTime;

  /**
   * Returns "Class/Test" for a test called by callExpr inside the current
   * class ("TestFoo()" -> "Class/TestFoo")
   */
  static std::string makeTestPath(const std::string &callExpr);

public:
  GOTestScope(Kind kind, const std::string &name);
  ~GOTestScope();

  /**
   * Sets the name of the test class whose tests are being run and resets the
   * per-class counters of shouldRun() calls
   */
  static void setCurrentClass(const std::string &className);

  /**
   * Switches the silent probing mode: shouldRun() only counts the calls and
   * the selected tests of the current class and neither prints nor runs
   * anything
   */
  static void setProbing(bool isProbing) { s_IsProbing = isProbing; }

  /**
   * Returns true if the current class has no tests selected by GOTestFilter
   * although it has GO_RUN_TEST calls (valid after a probing run of the class)
   */
  static bool hasNoSelectedTests() {
    return s_NCalls > 0 && s_NSelectedInClass == 0;
  }

  /**
   * Returns the finished scopes in the order of their completion
   */
  static const std::vector<Record> &getRecords() { return s_records; }

  /**
   * Returns true if the test called by callExpr must be run: it is selected
   * by GOTestFilter and the tests are not only listed. In the list-only mode
   * prints the path of a selected test instead of running it. In the probing
   * mode only counts the calls.
   * @param callExpr the call expression like "TestFoo()"
   */
  static bool shouldRun(const std::string &callExpr);
};

/**
 * Runs callExpr as a Test, logged via GOTestScope, if it is selected by
 * GOTestFilter. The test name is derived from the call expression itself.
 * Called without a trailing semicolon, like the other declaration-style
 * macros in this codebase.
 */
#define GO_RUN_TEST(callExpr)                                                  \
  if (GOTestScope::shouldRun(#callExpr)) {                                     \
    GOTestScope goTestScope_(GOTestScope::TEST, #callExpr);                    \
                                                                               \
    callExpr;                                                                  \
  }

#endif /* GOTESTSCOPE_H */
