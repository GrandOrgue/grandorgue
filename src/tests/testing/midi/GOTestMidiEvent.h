/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTMIDIEVENT_H
#define GOTESTMIDIEVENT_H

#include "GOTest.h"

/**
 * Tests GOMidiEvent::FromMidi() with SysEx messages
 */
class GOTestMidiEvent : public GOTest {

private:
  static const std::string TEST_NAME;

  void TestRodgersStopChange();
  void TestRodgersStopChangeEmptyData();
  void TestRodgersStopChangeTooShort();

public:
  std::string GetName() override { return TEST_NAME; }
  void run() override;
};

#endif /* GOTESTMIDIEVENT_H */
