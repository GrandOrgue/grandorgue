/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTPATTERNLIST_H
#define GOMIDIEVENTPATTERNLIST_H

#include <algorithm>
#include <vector>

#include "GOMidiBasePatternList.h"

template <
  class MidiType,
  class MidiEventPattern,
  typename = std::enable_if_t<
    std::is_base_of<GOMidiEventPattern, MidiEventPattern>::value>>
class GOMidiEventPatternList : public GOMidiBasePatternList {
protected:
  MidiType m_type;
  std::vector<MidiEventPattern> m_events;

public:
  GOMidiEventPatternList(MidiType type) : m_type(type) {}
  virtual ~GOMidiEventPatternList() {}

  MidiType GetType() const { return m_type; }

  unsigned GetEventCount() const override { return m_events.size(); }

  bool IsMidiConfigured() const { return !m_events.empty(); }

  void ClearEvents() { m_events.clear(); }

  const MidiEventPattern &GetEvent(unsigned index) const {
    return m_events[index];
  }

  const GOMidiEventPattern &GetBasePattern(unsigned index) const override {
    return GetEvent(index);
  }

  MidiEventPattern &GetEvent(unsigned index) { return m_events[index]; }

  unsigned AddNewEvent() {
    m_events.emplace_back();
    return m_events.size() - 1;
  }

  void DeleteEvent(unsigned index) { m_events.erase(m_events.begin() + index); }

  /**
   * Assign the new list to the current one
   * @param newList the lnew list to assign
   * @return whether the list is changed
   */

  bool RenewFrom(const GOMidiEventPatternList &newList) {
    std::vector<MidiEventPattern> nonEmptyEvents;

    // copy not empty events.
    std::copy_if(
      newList.m_events.begin(),
      newList.m_events.end(),
      std::back_inserter(nonEmptyEvents),
      [](const MidiEventPattern &x) { return !x.IsEmpty(); });

    bool result = newList.m_type != m_type || nonEmptyEvents != m_events;

    if (result) {
      m_type = newList.m_type;
      m_events = nonEmptyEvents;
    }
    return result;
  }
};

#endif /* GOMIDIEVENTPATTERNLIST_H */
