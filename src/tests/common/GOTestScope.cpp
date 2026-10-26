/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestScope.h"

#include <iostream>

#include "GOTestFilter.h"

int GOTestScope::s_Depth = 0;
std::string GOTestScope::s_CurrentClass;
std::vector<GOTestScope::Record> GOTestScope::s_records;
bool GOTestScope::s_IsProbing = false;
unsigned GOTestScope::s_NCalls = 0;
unsigned GOTestScope::s_NSelectedInClass = 0;

void GOTestScope::setCurrentClass(const std::string &className) {
  s_CurrentClass = className;
  s_NCalls = 0;
  s_NSelectedInClass = 0;
}

static std::string indent_for(int depth) { return std::string(2 * depth, ' '); }

std::string GOTestScope::makeTestPath(const std::string &callExpr) {
  return s_CurrentClass + "/" + callExpr.substr(0, callExpr.find('('));
}

bool GOTestScope::shouldRun(const std::string &callExpr) {
  GOTestFilter &filter = GOTestFilter::getInstance();
  const std::string path = makeTestPath(callExpr);
  const bool isSelected = filter.IsTestSelected(
    s_CurrentClass, path.substr(s_CurrentClass.size() + 1));

  s_NCalls++;
  if (isSelected) {
    s_NSelectedInClass++;
    if (s_IsProbing)
      // counted while probing, before setUp() that may fail
      filter.CountSelected();
    else if (filter.IsListOnly())
      std::cout << path << std::endl;
  }
  return isSelected && !s_IsProbing && !filter.IsListOnly();
}

GOTestScope::GOTestScope(Kind kind, const std::string &name)
  : m_Kind(kind),
    m_Name(name),
    m_path(kind == TEST_CLASS ? name : makeTestPath(name)),
    m_StartTime(std::chrono::steady_clock::now()) {
  if (m_Kind == TEST_CLASS)
    setCurrentClass(m_Name);
  std::cout << indent_for(s_Depth)
            << (m_Kind == TEST_CLASS ? "Test class: " : "Test: ") << m_Name
            << std::endl;
  s_Depth++;
}

GOTestScope::~GOTestScope() {
  const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - m_StartTime)
                           .count();

  s_records.push_back({m_Kind, m_path, elapsedMs});
  s_Depth--;
  std::cout << indent_for(s_Depth) << "done: " << m_Name << " (" << elapsedMs
            << " ms)" << std::endl;
}
