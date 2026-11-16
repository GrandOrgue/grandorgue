/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundItemAccessor.h"

#include <cstdint>
#include <cstring>
#include <vector>

#include "sound/playing/GOSoundItemAccessor.h"

#include "GOTestScope.h"
#include "GOTestUtils.h"
#include "GOWave.h"

const std::string GOTestSoundItemAccessor::TEST_NAME
  = "GOTestSoundItemAccessor";

static constexpr unsigned N_CHANNELS = 2;
static constexpr unsigned N_FRAMES = 5;

void GOTestSoundItemAccessor::TestIntegerRoundTrip() {
  for (const unsigned nBits : {8, 16, 24, 32}) {
    const GOSoundItemAccessor accessor(nBits, false, N_CHANNELS);
    const int64_t maxValue = (int64_t(1) << (nBits - 1)) - 1;
    const int values[N_FRAMES] = {int(-maxValue - 1), -1, 0, 1, int(maxValue)};
    std::vector<unsigned char> data(N_FRAMES * N_CHANNELS * nBits / 8, 0x5A);

    // Channel 1 gets the values in the reverse order
    for (unsigned frameI = 0; frameI < N_FRAMES; frameI++) {
      accessor.SetIntItem(data.data(), frameI, 0, values[frameI]);
      accessor.SetIntItem(
        data.data(), frameI, 1, values[N_FRAMES - 1 - frameI]);
    }
    for (unsigned frameI = 0; frameI < N_FRAMES; frameI++)
      for (unsigned channelI = 0; channelI < N_CHANNELS; channelI++) {
        const int value = values[channelI ? N_FRAMES - 1 - frameI : frameI];

        GOAssert(
          accessor.GetIntItem(data.data(), frameI, channelI) == value,
          "an integer item must be read back in int units");
        GOAssert(
          accessor.GetFloatItem(data.data(), frameI, channelI) == (float)value,
          "an integer item must be read in float units as the same number");
      }
  }
}

void GOTestSoundItemAccessor::TestFloatRoundTrip() {
  const GOSoundItemAccessor accessor(32, true, 1);
  const float values[] = {1.5f, -1.5f, 1e-9f, 0.1f};
  const int intValues[] = {-(1 << 23), -1, 0, 1, 12345, 1 << 23};
  float item;

  for (const float value : values) {
    accessor.SetFloatItem(&item, 0, 0, value);
    GOAssert(
      accessor.GetFloatItem(&item, 0, 0) == value,
      "a float item must be stored as it is");
    GOAssert(
      accessor.GetIntItem(&item, 0, 0) == GOWave::floatToIntQ23(value),
      "a float item must be read in int units as Q23");
  }
  for (const int value : intValues) {
    accessor.SetIntItem(&item, 0, 0, value);
    GOAssert(
      accessor.GetFloatItem(&item, 0, 0) == GOWave::intQ23ToFloat(value),
      "an int written to a float item must be converted from Q23");
  }
  accessor.SetFloatItem(&item, 0, 0, 2.0f);
  GOAssert(
    accessor.GetIntItem(&item, 0, 0) == (1 << 23),
    "an int read from a float item must be clamped to full scale");
}

void GOTestSoundItemAccessor::TestIntegerTruncatesFloat() {
  const GOSoundItemAccessor accessor(16, false, 1);
  int16_t item;

  accessor.SetFloatItem(&item, 0, 0, 3.9f);
  GOAssert(
    accessor.GetIntItem(&item, 0, 0) == 3,
    "a positive float must be truncated towards zero");
  accessor.SetFloatItem(&item, 0, 0, -3.9f);
  GOAssert(
    accessor.GetIntItem(&item, 0, 0) == -3,
    "a negative float must be truncated towards zero");
}

void GOTestSoundItemAccessor::TestUnalignedData() {
  const GOSoundItemAccessor accessor(32, true, 1);
  const float values[3] = {1.5f, -1e-9f, 0.1f};
  unsigned char data[1 + sizeof(values)];

  memset(data, 0xA5, sizeof(data));
  for (unsigned frameI = 0; frameI < 3; frameI++)
    accessor.SetFloatItem(data + 1, frameI, 0, values[frameI]);
  GOAssert(
    memcmp(data + 1, values, sizeof(values)) == 0,
    "a float must be stored as its bytes at an unaligned address");
  GOAssert(data[0] == 0xA5, "the byte before the data must stay intact");
  for (unsigned frameI = 0; frameI < 3; frameI++)
    GOAssert(
      accessor.GetFloatItem(data + 1, frameI, 0) == values[frameI],
      "an unaligned float must be read back");
}

void GOTestSoundItemAccessor::run() {
  GO_RUN_TEST(TestIntegerRoundTrip())
  GO_RUN_TEST(TestFloatRoundTrip())
  GO_RUN_TEST(TestIntegerTruncatesFloat())
  GO_RUN_TEST(TestUnalignedData())
}
