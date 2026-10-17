/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestFilter.h"

static GOTestFilter instance;

GOTestFilter &GOTestFilter::getInstance() { return instance; }

void GOTestFilter::AddPattern(const std::string &pattern) {
  const bool isExclude = !pattern.empty() && pattern[0] == '!';
  const std::string body = isExclude ? pattern.substr(1) : pattern;
  const size_t slashPos = body.find('/');
  Pattern parsed;

  parsed.m_ClassRegex = std::regex(body.substr(0, slashPos));
  if (slashPos != std::string::npos)
    parsed.m_TestRegex = std::regex(body.substr(slashPos + 1));
  (isExclude ? m_excludes : m_includes).push_back(parsed);
}

bool GOTestFilter::IsMatching(
  const Pattern &pattern,
  const std::string &className,
  const std::string *pTestName) const {
  return std::regex_search(className, pattern.m_ClassRegex)
    && (!pattern.m_TestRegex || !pTestName
        || std::regex_search(*pTestName, *pattern.m_TestRegex));
}

bool GOTestFilter::IsClassSelected(const std::string &className) const {
  bool isOk = m_includes.empty();

  for (const Pattern &include : m_includes)
    isOk = isOk || IsMatching(include, className, nullptr);
  for (const Pattern &exclude : m_excludes)
    // only a whole-class exclude rejects the class
    isOk = isOk
      && (exclude.m_TestRegex || !IsMatching(exclude, className, nullptr));
  return isOk;
}

bool GOTestFilter::IsTestSelected(
  const std::string &className, const std::string &testName) const {
  bool isOk = m_includes.empty();

  for (const Pattern &include : m_includes)
    isOk = isOk || IsMatching(include, className, &testName);
  for (const Pattern &exclude : m_excludes)
    isOk = isOk && !IsMatching(exclude, className, &testName);
  return isOk;
}
