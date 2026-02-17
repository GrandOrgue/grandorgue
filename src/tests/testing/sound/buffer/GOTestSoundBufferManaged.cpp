/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferManaged.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <numeric>
#include <vector>

#include "sound/buffer/GOSoundBuffer.h"
#include "sound/buffer/GOSoundBufferManaged.h"

const std::string GOTestSoundBufferManaged::TEST_NAME
  = "GOTestSoundBufferManaged";

void GOTestSoundBufferManaged::TestDefaultConstructor() {
  GOSoundBufferManaged buffer;

  GOAssert(!buffer.isValid(), "Default constructed buffer should be invalid");
  GOAssert(
    buffer.GetNChannels() == 0,
    std::format(
      "Default constructed buffer should have 0 channels (got: {})",
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == 0,
    std::format(
      "Default constructed buffer should have 0 frames (got: {})",
      buffer.GetNFrames()));
}

void GOTestSoundBufferManaged::TestConstructorWithDimensions() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  GOSoundBufferManaged buffer(nChannels, nFrames);

  GOAssert(buffer.isValid(), "Buffer with dimensions should be valid");
  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "Buffer should have {} channels (got: {})",
      nChannels,
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "Buffer should have {} frames (got: {})", nFrames, buffer.GetNFrames()));

  // Check that memory is accessible (no crash)
  GOSoundBuffer::Item *data = buffer.GetData();
  for (unsigned i = 0; i < totalItems; ++i) {
    data[i] = static_cast<float>(i);
  }

  // Verify we can read back
  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      data[i] == static_cast<float>(i),
      std::format("Data unit {} should be {} (got: {})", i, i, data[i]));
  }
}

void GOTestSoundBufferManaged::TestCopyConstructorFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(totalItems);
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);
  GOSoundBufferManaged buffer(sourceBuffer);

  GOAssert(buffer.isValid(), "Copied buffer should be valid");
  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "Copied buffer should have {} channels (got: {})",
      nChannels,
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "Copied buffer should have {} frames (got: {})",
      nFrames,
      buffer.GetNFrames()));

  // Verify data was copied
  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == sourceData[i],
      std::format(
        "Unit {} should be {} (got: {})",
        i,
        sourceData[i],
        buffer.GetData()[i]));
  }

  // Verify it's a deep copy (modifying source doesn't affect buffer)
  sourceData[0] = 999.0f;
  GOAssert(
    buffer.GetData()[0] != 999.0f,
    "Buffer should have independent copy of data");
}

void GOTestSoundBufferManaged::TestCopyConstructorFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 1);
  }

  GOSoundBufferManaged buffer(source);

  GOAssert(buffer.isValid(), "Copied buffer should be valid");

  // Verify data was copied
  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == sourceData[i],
      std::format(
        "Unit {} should be {} (got: {})",
        i,
        sourceData[i],
        buffer.GetData()[i]));
  }

  // Verify it's a deep copy
  sourceData[0] = 999.0f;
  GOAssert(
    buffer.GetData()[0] != 999.0f,
    "Buffer should have independent copy of data");
}

void GOTestSoundBufferManaged::TestMoveConstructor() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer::Item *originalPtr = source.GetData();

  GOSoundBufferManaged buffer(std::move(source));

  // Buffer should have the data
  GOAssert(buffer.isValid(), "Moved-to buffer should be valid");
  GOAssert(
    buffer.GetData() == originalPtr,
    "Moved-to buffer should have same data pointer");

  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == static_cast<float>(i + 1),
      std::format(
        "Unit {} should be {} (got: {})", i, i + 1, buffer.GetData()[i]));
  }

  // Source should be empty
  GOAssert(!source.isValid(), "Moved-from buffer should be invalid");
  GOAssert(
    source.GetData() == nullptr,
    "Moved-from buffer should have null data pointer");
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(totalItems);
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 10);
  }

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);
  GOSoundBufferManaged buffer(1, 1); // Different size initially

  buffer = sourceBuffer;

  GOAssert(buffer.isValid(), "Assigned buffer should be valid");
  GOAssert(
    buffer.GetNChannels() == nChannels,
    std::format(
      "Assigned buffer should have {} channels (got: {})",
      nChannels,
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == nFrames,
    std::format(
      "Assigned buffer should have {} frames (got: {})",
      nFrames,
      buffer.GetNFrames()));

  // Verify data was copied
  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == sourceData[i],
      std::format(
        "Unit {} should be {} (got: {})",
        i,
        sourceData[i],
        buffer.GetData()[i]));
  }
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 20);
  }

  GOSoundBufferManaged buffer(1, 2); // Different size initially

  buffer = source;

  GOAssert(buffer.isValid(), "Assigned buffer should be valid");

  // Verify data was copied
  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == sourceData[i],
      std::format(
        "Unit {} should be {} (got: {})",
        i,
        sourceData[i],
        buffer.GetData()[i]));
  }

  // Verify it's a deep copy
  sourceData[0] = 999.0f;
  GOAssert(
    buffer.GetData()[0] != 999.0f,
    "Buffer should have independent copy of data");
}

void GOTestSoundBufferManaged::TestMoveAssignment() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 30);
  }

  GOSoundBuffer::Item *originalPtr = source.GetData();
  GOSoundBufferManaged buffer(1, 1); // Different size initially

  buffer = std::move(source);

  // Buffer should have the data
  GOAssert(buffer.isValid(), "Moved-to buffer should be valid");
  GOAssert(
    buffer.GetData() == originalPtr,
    "Moved-to buffer should have same data pointer");

  for (unsigned i = 0; i < totalItems; ++i) {
    GOAssert(
      buffer.GetData()[i] == static_cast<float>(i + 30),
      std::format(
        "Unit {} should be {} (got: {})", i, i + 30, buffer.GetData()[i]));
  }

  // Source should be empty
  GOAssert(!source.isValid(), "Moved-from buffer should be invalid");
  GOAssert(
    source.GetData() == nullptr,
    "Moved-from buffer should have null data pointer");
}

void GOTestSoundBufferManaged::TestResize() {
  GOSoundBufferManaged buffer(2, 3);
  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Fill with data
  for (unsigned i = 0; i < 6; ++i) {
    oldData[i] = static_cast<float>(i + 1);
  }

  // Resize to larger
  buffer.Resize(2, 5);
  GOAssert(
    buffer.GetNFrames() == 5,
    std::format(
      "Buffer should have 5 frames after resize (got: {})",
      buffer.GetNFrames()));
  GOAssert(
    buffer.GetData() != oldData,
    "Data pointer should change after resize to larger");

  // Resize to smaller
  oldData = buffer.GetData();
  buffer.Resize(1, 2);
  GOAssert(
    buffer.GetNChannels() == 1,
    std::format(
      "Buffer should have 1 channel after resize (got: {})",
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == 2,
    std::format(
      "Buffer should have 2 frames after resize (got: {})",
      buffer.GetNFrames()));
  GOAssert(
    buffer.GetData() != oldData,
    "Data pointer should change after resize to different channel count");

  // Resize to zero
  buffer.Resize(0, 0);
  GOAssert(!buffer.isValid(), "Buffer should be invalid after resize to zero");
  GOAssert(
    buffer.GetNChannels() == 0,
    std::format(
      "Buffer should have 0 channels after zero resize (got: {})",
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == 0,
    std::format(
      "Buffer should have 0 frames after zero resize (got: {})",
      buffer.GetNFrames()));
}

void GOTestSoundBufferManaged::TestResizeNoReallocation() {
  GOSoundBufferManaged buffer(2, 3); // 2*3 = 6 units
  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Fill with data
  for (unsigned i = 0; i < 6; ++i) {
    oldData[i] = static_cast<float>(i + 1);
  }

  // Resize to 3x2 (same total units)
  buffer.Resize(3, 2); // 3*2 = 6 units
  GOAssert(
    buffer.GetData() == oldData,
    "Data pointer should NOT change when total units unchanged");
  GOAssert(
    buffer.GetNChannels() == 3,
    std::format(
      "Buffer should have 3 channels after resize (got: {})",
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == 2,
    std::format(
      "Buffer should have 2 frames after resize (got: {})",
      buffer.GetNFrames()));

  // Data should be preserved
  for (unsigned i = 0; i < 6; ++i) {
    GOAssert(
      oldData[i] == static_cast<float>(i + 1),
      std::format("Unit {} should be preserved (got: {})", i + 1, oldData[i]));
  }
}

void GOTestSoundBufferManaged::TestSwap() {
  GOSoundBufferManaged buffer1(2, 3);
  GOSoundBufferManaged buffer2(3, 2);

  GOSoundBuffer::Item *data1 = buffer1.GetData();
  GOSoundBuffer::Item *data2 = buffer2.GetData();

  // Fill with different data
  for (unsigned i = 0; i < 6; ++i) {
    data1[i] = static_cast<float>(i + 100);
    data2[i] = static_cast<float>(i + 200);
  }

  const unsigned channels1 = buffer1.GetNChannels();
  const unsigned frames1 = buffer1.GetNFrames();
  const unsigned channels2 = buffer2.GetNChannels();
  const unsigned frames2 = buffer2.GetNFrames();

  swap(buffer1, buffer2);

  // Check that dimensions are swapped
  GOAssert(
    buffer1.GetNChannels() == channels2,
    std::format(
      "Buffer1 should have {} channels after swap (got: {})",
      channels2,
      buffer1.GetNChannels()));
  GOAssert(
    buffer1.GetNFrames() == frames2,
    std::format(
      "Buffer1 should have {} frames after swap (got: {})",
      frames2,
      buffer1.GetNFrames()));
  GOAssert(
    buffer2.GetNChannels() == channels1,
    std::format(
      "Buffer2 should have {} channels after swap (got: {})",
      channels1,
      buffer2.GetNChannels()));
  GOAssert(
    buffer2.GetNFrames() == frames1,
    std::format(
      "Buffer2 should have {} frames after swap (got: {})",
      frames1,
      buffer2.GetNFrames()));

  // Check that data pointers are swapped
  GOAssert(
    buffer1.GetData() == data2,
    "Buffer1 should have buffer2's data after swap");
  GOAssert(
    buffer2.GetData() == data1,
    "Buffer2 should have buffer1's data after swap");

  // Check that data content is swapped
  for (unsigned i = 0; i < 6; ++i) {
    GOAssert(
      buffer1.GetData()[i] == static_cast<float>(i + 200),
      std::format(
        "Buffer1 unit {} should be {} (got: {})",
        i,
        i + 200,
        buffer1.GetData()[i]));
    GOAssert(
      buffer2.GetData()[i] == static_cast<float>(i + 100),
      std::format(
        "Buffer2 unit {} should be {} (got: {})",
        i,
        i + 100,
        buffer2.GetData()[i]));
  }
}

void GOTestSoundBufferManaged::TestComplexOperations() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned totalItems = nChannels * nFrames;

  // Create source buffer with external memory
  std::vector<GOSoundBuffer::Item> sourceData(totalItems);
  for (unsigned i = 0; i < totalItems; ++i) {
    sourceData[i] = static_cast<float>(i + 1);
  }

  GOSoundBuffer sourceBuffer(sourceData.data(), nChannels, nFrames);

  // Create managed buffer and copy data
  GOSoundBufferManaged buffer(sourceBuffer);

  // Modify managed buffer
  buffer.AddFrom(sourceBuffer, 0.5f); // buffer = 1.5 * source

  // Create another managed buffer
  GOSoundBufferManaged buffer2(2, 3);
  buffer2.FillWithSilence();
  buffer2.AddFrom(buffer, 2.0f); // buffer2 = 3.0 * source

  // Verify calculations
  for (unsigned i = 0; i < totalItems; ++i) {
    float expected = static_cast<float>(i + 1) * 3.0f;
    const float epsilon = 1e-6f;
    float diff = std::abs(buffer2.GetData()[i] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "Unit {} should be {} (got: {}, diff: {})",
        i,
        expected,
        buffer2.GetData()[i],
        diff));
  }
}

void GOTestSoundBufferManaged::TestSubBufferCompatibility() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GOSoundBufferManaged buffer(nChannels, nFrames);

  // Get a sub-buffer (mutable)
  GOSoundBufferMutable subBuffer = buffer.GetSubBuffer(1, 2);

  GOAssert(subBuffer.isValid(), "Sub-buffer should be valid");
  GOAssert(
    subBuffer.GetNFrames() == 2,
    std::format(
      "Sub-buffer should have 2 frames (got: {})", subBuffer.GetNFrames()));

  // Modify through sub-buffer
  subBuffer.FillWithSilence();

  // Verify modification affected the main buffer
  GOSoundBuffer::Item *mainData = buffer.GetData();
  GOAssert(
    mainData[2] == 0.0f && mainData[3] == 0.0f,
    "First channel of second frame should be zeroed");
  GOAssert(
    mainData[4] == 0.0f && mainData[5] == 0.0f,
    "Second channel of third frame should be zeroed");
}

void GOTestSoundBufferManaged::TestInvalidBufferOperations() {
  GOSoundBufferManaged buffer;

  // Operations on invalid buffer should be safe or assert
  GOAssert(!buffer.isValid(), "Default buffer should be invalid");
  GOAssert(
    buffer.GetNChannels() == 0,
    std::format(
      "Invalid buffer should have 0 channels (got: {})",
      buffer.GetNChannels()));
  GOAssert(
    buffer.GetNFrames() == 0,
    std::format(
      "Invalid buffer should have 0 frames (got: {})", buffer.GetNFrames()));
  GOAssert(
    buffer.GetData() == nullptr,
    "Invalid buffer should have null data pointer");

  // Resize should work
  buffer.Resize(2, 3);
  GOAssert(buffer.isValid(), "Buffer should be valid after resize");
}

void GOTestSoundBufferManaged::run() {
  TestDefaultConstructor();
  TestConstructorWithDimensions();
  TestCopyConstructorFromBuffer();
  TestCopyConstructorFromManaged();
  TestMoveConstructor();
  TestCopyAssignmentFromBuffer();
  TestCopyAssignmentFromManaged();
  TestMoveAssignment();
  TestResize();
  TestResizeNoReallocation();
  TestSwap();
  TestComplexOperations();
  TestSubBufferCompatibility();
  TestInvalidBufferOperations();
}
