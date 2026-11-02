/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTFILTER_H
#define GOTESTFILTER_H

#include <optional>
#include <regex>
#include <string>
#include <vector>

/**
 * Selection of tests by the patterns given in the command line. The shared
 * instance returned by getInstance() is used from GO_RUN_TEST everywhere in
 * the tests.
 *
 * A pattern is "ClassRegex" or "ClassRegex/TestRegex". Both parts are
 * ECMAScript regular expressions searched (not fully matched) in the class
 * name and in the test name. A pattern without "/" selects whole classes, a
 * pattern with "/" selects only the matching tests of the matching classes.
 * A pattern starting with "!" excludes instead of includes. If there are no
 * include patterns then everything is included.
 */
class GOTestFilter {
private:
  struct Pattern {
    std::regex m_ClassRegex;
    std::optional<std::regex> m_TestRegex; // absent: the whole class
  };

  std::vector<Pattern> m_includes;
  std::vector<Pattern> m_excludes;
  bool m_IsListOnly = false;
  unsigned m_NSelected = 0;

  /**
   * Checks if the pattern matches the class and (if pTestName is not null)
   * the test. A class-level pattern matches all tests of the matching class.
   * If pTestName is null then a test-level pattern matches the class if its
   * class part does.
   */
  bool IsMatching(
    const Pattern &pattern,
    const std::string &className,
    const std::string *pTestName) const;

public:
  GOTestFilter() {}

  /**
   * Returns the shared instance
   */
  static GOTestFilter &getInstance();

  /**
   * Adds one pattern. A comma has no special meaning and is a part of the
   * regular expression
   * @param pattern "[!]ClassRegex[/TestRegex]"
   * @throws std::regex_error if a regular expression is not valid
   */
  void AddPattern(const std::string &pattern);

  /**
   * Sets whether the tests are only listed and not run
   */
  void SetListOnly(bool isListOnly) { m_IsListOnly = isListOnly; }

  /**
   * Returns true if the tests are only listed and not run
   */
  bool IsListOnly() const { return m_IsListOnly; }

  /**
   * Returns true if some test of the class may be selected, so the class
   * must be set up
   */
  bool IsClassSelected(const std::string &className) const;

  /**
   * Returns true if the test of the class is selected
   */
  bool IsTestSelected(
    const std::string &className, const std::string &testName) const;

  /**
   * Counts a selected test
   */
  void CountSelected() { m_NSelected++; }

  /**
   * Returns the number of the selected tests counted by CountSelected()
   */
  unsigned GetNSelected() const { return m_NSelected; }
};

#endif /* GOTESTFILTER_H */
