/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOMIDIEVENTPATTERNLIST_H
#define GOMIDIEVENTPATTERNLIST_H

#include <vector>

template <class MidiType, class MidiEventPattern> class GOMidiEventPatternList {
protected:
  MidiType m_type;
  std::vector<MidiEventPattern> m_events;

public:
  GOMidiEventPatternList(MidiType type) : m_type(type) {}
  virtual ~GOMidiEventPatternList() {}

  MidiType GetType() const { return m_type; }

  unsigned GetEventCount() const { return m_events.size(); }

  const MidiEventPattern &GetEvent(unsigned index) const {
    return m_events[index];
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
    bool result = newList.m_type != m_type || newList.m_events != m_events;

    if (result)
      *this = newList;
    return result;
  }
};

#endif /* GOMIDIEVENTPATTERNLIST_H */
