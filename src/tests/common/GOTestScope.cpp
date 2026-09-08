/*
 * Copyright 2023-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestScope.h"

#include <iostream>

int GOTestScope::s_Depth = 0;

static std::string indent_for(int depth) { return std::string(2 * depth, ' '); }

GOTestScope::GOTestScope(Kind kind, const std::string &name)
  : m_Kind(kind), m_Name(name), m_StartTime(std::chrono::steady_clock::now()) {
  std::cout << indent_for(s_Depth)
            << (m_Kind == TEST_CLASS ? "Test class: " : "Test: ") << m_Name
            << std::endl;
  s_Depth++;
}

GOTestScope::~GOTestScope() {
  const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - m_StartTime)
                           .count();

  s_Depth--;
  std::cout << indent_for(s_Depth) << "done: " << m_Name << " (" << elapsedMs
            << " ms)" << std::endl;
}
