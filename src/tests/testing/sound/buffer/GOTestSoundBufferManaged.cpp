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

void GOTestSoundBufferManaged::AssertDataEquals(
  const std::string &context,
  const GOSoundBuffer::Item *expectedData,
  unsigned nItems,
  const GOSoundBuffer &buffer) {
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      buffer.GetData()[itemI] == expectedData[itemI],
      std::format(
        "{}: item {} got: {}, expected: {}",
        context,
        itemI,
        buffer.GetData()[itemI],
        expectedData[itemI]));
}

void GOTestSoundBufferManaged::TestDeepCopy(
  const std::string &context,
  GOSoundBuffer::Item *sourceData,
  const GOSoundBuffer &buffer) {
  sourceData[0] = 999.0f;
  GOAssert(
    buffer.GetData()[0] != 999.0f,
    context + ": buffer should have independent copy of data");
}

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
  const unsigned nItems = nChannels * nFrames;

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

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    data[itemI] = static_cast<float>(itemI);

  // Verify we can read back
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      data[itemI] == static_cast<float>(itemI),
      std::format(
        "ConstructorWithDimensions: item {} got: {}, expected: {}",
        itemI,
        data[itemI],
        static_cast<float>(itemI)));
}

void GOTestSoundBufferManaged::TestCopyConstructorFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 1);

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
  AssertDataEquals(
    "CopyConstructorFromBuffer", sourceData.data(), nItems, buffer);

  // Verify it's a deep copy (modifying source doesn't affect buffer)
  TestDeepCopy("CopyConstructorFromBuffer", sourceData.data(), buffer);
}

void GOTestSoundBufferManaged::TestCopyConstructorFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 1);

  GOSoundBufferManaged buffer(source);

  GOAssert(buffer.isValid(), "Copied buffer should be valid");

  // Verify data was copied
  AssertDataEquals("CopyConstructorFromManaged", sourceData, nItems, buffer);

  // Verify it's a deep copy
  TestDeepCopy("CopyConstructorFromManaged", sourceData, buffer);
}

void GOTestSoundBufferManaged::TestMoveConstructor() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 1);

  GOSoundBuffer::Item *originalPtr = source.GetData();

  GOSoundBufferManaged buffer(std::move(source));

  // Buffer should have the data
  GOAssert(buffer.isValid(), "Moved-to buffer should be valid");
  GOAssert(
    buffer.GetData() == originalPtr,
    "Moved-to buffer should have same data pointer");

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      buffer.GetData()[itemI] == static_cast<float>(itemI + 1),
      std::format(
        "MoveConstructor: item {} got: {}, expected: {}",
        itemI,
        buffer.GetData()[itemI],
        static_cast<float>(itemI + 1)));

  // Source should be empty
  GOAssert(!source.isValid(), "Moved-from buffer should be invalid");
  GOAssert(
    source.GetData() == nullptr,
    "Moved-from buffer should have null data pointer");
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromBuffer() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 10);

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
  AssertDataEquals(
    "CopyAssignmentFromBuffer", sourceData.data(), nItems, buffer);
}

void GOTestSoundBufferManaged::TestCopyAssignmentFromManaged() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 20);

  GOSoundBufferManaged buffer(1, 2); // Different size initially

  buffer = source;

  GOAssert(buffer.isValid(), "Assigned buffer should be valid");

  // Verify data was copied
  AssertDataEquals("CopyAssignmentFromManaged", sourceData, nItems, buffer);

  // Verify it's a deep copy
  TestDeepCopy("CopyAssignmentFromManaged", sourceData, buffer);
}

void GOTestSoundBufferManaged::TestMoveAssignment() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  GOSoundBufferManaged source(nChannels, nFrames);
  GOSoundBuffer::Item *sourceData = source.GetData();

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 30);

  GOSoundBuffer::Item *originalPtr = source.GetData();
  GOSoundBufferManaged buffer(1, 1); // Different size initially

  buffer = std::move(source);

  // Buffer should have the data
  GOAssert(buffer.isValid(), "Moved-to buffer should be valid");
  GOAssert(
    buffer.GetData() == originalPtr,
    "Moved-to buffer should have same data pointer");

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      buffer.GetData()[itemI] == static_cast<float>(itemI + 30),
      std::format(
        "MoveAssignment: item {} got: {}, expected: {}",
        itemI,
        buffer.GetData()[itemI],
        static_cast<float>(itemI + 30)));

  // Source should be empty
  GOAssert(!source.isValid(), "Moved-from buffer should be invalid");
  GOAssert(
    source.GetData() == nullptr,
    "Moved-from buffer should have null data pointer");
}

void GOTestSoundBufferManaged::TestResize() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  GOSoundBufferManaged buffer(nChannels, nFrames);
  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Fill with data
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    oldData[itemI] = static_cast<float>(itemI + 1);

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
  const unsigned nItems = 6; // 2*3 = 3*2

  GOSoundBufferManaged buffer(2, 3);
  GOSoundBuffer::Item *oldData = buffer.GetData();

  // Fill with data
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    oldData[itemI] = static_cast<float>(itemI + 1);

  // Resize to 3x2 (same total items)
  buffer.Resize(3, 2);
  GOAssert(
    buffer.GetData() == oldData,
    "Data pointer should NOT change when total items unchanged");
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
  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    GOAssert(
      oldData[itemI] == static_cast<float>(itemI + 1),
      std::format(
        "ResizeNoReallocation: item {} got: {}, expected: {}",
        itemI,
        oldData[itemI],
        static_cast<float>(itemI + 1)));
}

void GOTestSoundBufferManaged::TestSwap() {
  const unsigned nChannels1 = 2;
  const unsigned nFrames1 = 3;
  const unsigned nChannels2 = 3;
  const unsigned nFrames2 = 2;
  const unsigned nItems = nChannels1 * nFrames1; // = nChannels2 * nFrames2

  GOSoundBufferManaged buffer1(nChannels1, nFrames1);
  GOSoundBufferManaged buffer2(nChannels2, nFrames2);
  GOSoundBuffer::Item *data1 = buffer1.GetData();
  GOSoundBuffer::Item *data2 = buffer2.GetData();

  // Fill with different data
  for (unsigned itemI = 0; itemI < nItems; ++itemI) {
    data1[itemI] = static_cast<float>(itemI + 100);
    data2[itemI] = static_cast<float>(itemI + 200);
  }

  swap(buffer1, buffer2);

  // Check that dimensions are swapped
  GOAssert(
    buffer1.GetNChannels() == nChannels2,
    std::format(
      "Buffer1 should have {} channels after swap (got: {})",
      nChannels2,
      buffer1.GetNChannels()));
  GOAssert(
    buffer1.GetNFrames() == nFrames2,
    std::format(
      "Buffer1 should have {} frames after swap (got: {})",
      nFrames2,
      buffer1.GetNFrames()));
  GOAssert(
    buffer2.GetNChannels() == nChannels1,
    std::format(
      "Buffer2 should have {} channels after swap (got: {})",
      nChannels1,
      buffer2.GetNChannels()));
  GOAssert(
    buffer2.GetNFrames() == nFrames1,
    std::format(
      "Buffer2 should have {} frames after swap (got: {})",
      nFrames1,
      buffer2.GetNFrames()));

  // Check that data pointers are swapped
  GOAssert(
    buffer1.GetData() == data2,
    "Buffer1 should have buffer2's data after swap");
  GOAssert(
    buffer2.GetData() == data1,
    "Buffer2 should have buffer1's data after swap");

  // Check that data content is swapped
  for (unsigned itemI = 0; itemI < nItems; ++itemI) {
    GOAssert(
      buffer1.GetData()[itemI] == static_cast<float>(itemI + 200),
      std::format(
        "Swap buffer1: item {} got: {}, expected: {}",
        itemI,
        buffer1.GetData()[itemI],
        static_cast<float>(itemI + 200)));
    GOAssert(
      buffer2.GetData()[itemI] == static_cast<float>(itemI + 100),
      std::format(
        "Swap buffer2: item {} got: {}, expected: {}",
        itemI,
        buffer2.GetData()[itemI],
        static_cast<float>(itemI + 100)));
  }
}

void GOTestSoundBufferManaged::TestComplexOperations() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;
  const unsigned nItems = nChannels * nFrames;

  // Create source buffer with external memory
  std::vector<GOSoundBuffer::Item> sourceData(nItems);

  for (unsigned itemI = 0; itemI < nItems; ++itemI)
    sourceData[itemI] = static_cast<float>(itemI + 1);

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
  for (unsigned itemI = 0; itemI < nItems; ++itemI) {
    float expected = static_cast<float>(itemI + 1) * 3.0f;
    const float epsilon = 1e-6f;
    float diff = std::abs(buffer2.GetData()[itemI] - expected);

    GOAssert(
      diff < epsilon,
      std::format(
        "ComplexOperations: item {} got: {}, expected: {} (diff: {})",
        itemI,
        buffer2.GetData()[itemI],
        expected,
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
