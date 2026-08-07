/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTESTSOUNDBUFFERBASE_H
#define GOTESTSOUNDBUFFERBASE_H

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "sound/buffer/GOSoundBuffer.h"

#include "GOTest.h"

/** Base class for GOSoundBuffer tests providing common helpers. */
class GOTestSoundBufferBase : public GOTest {
protected:
  /** Fills data[i] = static_cast<float>(i) + offset for i in [0, nItems). */
  static void fillWithSequential(
    GOSoundBuffer::Item *data, unsigned nItems, float offset);

  /**
   * Fills buffer.GetData()[i] = static_cast<float>(i) + offset for all i.
   * Works for any buffer exposing GetData()/GetNItems() (interleaved or
   * planar): a flat sequential fill doesn't care about channel layout.
   */
  template <typename BufferType>
  static void fillWithSequential(BufferType &buffer, float offset) {
    fillWithSequential(buffer.GetData(), buffer.GetNItems(), offset);
  }

  /** Asserts gotValue == expectedValue, reporting item index on failure. */
  void AssertItemEqual(
    const std::string &context,
    unsigned itemIndex,
    float expectedValue,
    float gotValue);

  /**
   * Asserts buffer.GetData()[i] == static_cast<float>(i) + offset for all i.
   * Works for any buffer exposing GetData()/GetNItems() (interleaved or
   * planar).
   */
  template <typename BufferType>
  void AssertSequentialData(
    const std::string &context, const BufferType &buffer, float offset) {
    for (unsigned n = buffer.GetNItems(), itemI = 0; itemI < n; ++itemI)
      AssertItemEqual(
        context,
        itemI,
        static_cast<float>(itemI) + offset,
        buffer.GetData()[itemI]);
  }

  /**
   * Asserts buffer dimensions and validity match expected values. Works for
   * any buffer exposing isValid()/GetNChannels()/GetNFrames() (interleaved
   * or planar).
   */
  template <typename BufferType>
  void AssertDimensions(
    const std::string &context,
    const BufferType &buffer,
    unsigned expectedNChannels,
    unsigned expectedNFrames) {
    const bool expectedValid = expectedNChannels > 0 && expectedNFrames > 0;

    GOAssert(
      buffer.isValid() == expectedValid,
      std::format(
        "{}: buffer should be {}",
        context,
        expectedValid ? "valid" : "invalid"));
    GOAssert(
      buffer.GetNChannels() == expectedNChannels,
      std::format(
        "{}: buffer should have {} channels (got: {})",
        context,
        expectedNChannels,
        buffer.GetNChannels()));
    GOAssert(
      buffer.GetNFrames() == expectedNFrames,
      std::format(
        "{}: buffer should have {} frames (got: {})",
        context,
        expectedNFrames,
        buffer.GetNFrames()));
  }

  /**
   * Asserts the basic validity rules common to every buffer type
   * (interleaved or planar): null data, 0 channels or 0 frames make a
   * buffer invalid; 1 frame with 2 channels is a valid, 2-item buffer. Both
   * GOSoundBuffer and GOSoundBufferPlanar (and their mutable/managed
   * variants) share the same (pData, nChannels, nFrames) constructor
   * signature, so this is generic over the concrete buffer type.
   * @param context Label for GOAssert failure messages
   */
  template <typename BufferType>
  void TestNullAndZeroDimensionInvalid(const std::string &context) {
    BufferType nullBuffer(nullptr, 2, 4);
    GOAssert(
      !nullBuffer.isValid(),
      context + ": buffer with null data pointer should be invalid");

    typename BufferType::Item dummyData[8];
    BufferType zeroChannelsBuffer(dummyData, 0, 4);
    GOAssert(
      !zeroChannelsBuffer.isValid(),
      context + ": buffer with 0 channels should be invalid");

    BufferType zeroFramesBuffer(dummyData, 2, 0);
    GOAssert(
      !zeroFramesBuffer.isValid(),
      context + ": buffer with 0 frames should be invalid");

    typename BufferType::Item singleFrame[2] = {1.0f, 2.0f};
    BufferType singleBuffer(singleFrame, 2, 1);
    GOAssert(
      singleBuffer.isValid(),
      context + ": buffer with 1 frame should be valid");
    GOAssert(
      singleBuffer.GetNItems() == 2,
      std::format(
        "{}: buffer with 2 channels and 1 frame should have 2 total items "
        "(got: {})",
        context,
        singleBuffer.GetNItems()));
  }

  // The following TestManagedXxx() methods each mirror one of the
  // Rule-of-5 / Resize() / Swap() cases common to every owning buffer
  // (GOSoundBufferManaged and GOSoundBufferPlanarManaged mirror each
  // other's constructors, assignment operators, Resize() and Swap() one for
  // one, via the shared GOSoundBufferManagedBase template), so each case is
  // written once instead of once per memory layout. ManagedType is the
  // owning buffer under test; ViewType is the read-only view type accepted
  // by its copy constructor/assignment (GOSoundBuffer or
  // GOSoundBufferPlanar).

  template <typename ManagedType>
  void TestManagedDefaultConstructor(const std::string &context) {
    ManagedType buffer;
    AssertDimensions(context, buffer, 0, 0);
  }

  template <typename ManagedType>
  void TestManagedConstructorWithDimensions(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType buffer(nChannels, nFrames);

    AssertDimensions(context, buffer, nChannels, nFrames);
    fillWithSequential(buffer, 0.0f);
    AssertSequentialData(context, buffer, 0.0f);
  }

  template <typename ManagedType, typename ViewType>
  void TestManagedCopyConstructorFromView(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    const unsigned nItems = nChannels * nFrames;
    std::vector<typename ManagedType::Item> sourceData(nItems);

    fillWithSequential(sourceData.data(), nItems, 1.0f);

    ViewType sourceBuffer(sourceData.data(), nChannels, nFrames);
    ManagedType buffer(sourceBuffer);

    AssertDimensions(context, buffer, nChannels, nFrames);
    AssertSequentialData(context, buffer, 1.0f);

    // Verify it's a deep copy
    sourceData[0] = 999.0f;
    GOAssert(
      buffer.GetData()[0] != 999.0f,
      context + ": buffer should have independent copy of data");
  }

  template <typename ManagedType>
  void TestManagedCopyConstructorFromManaged(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType source(nChannels, nFrames);

    fillWithSequential(source, 1.0f);

    ManagedType buffer(source);

    AssertDimensions(context, buffer, nChannels, nFrames);
    AssertSequentialData(context, buffer, 1.0f);

    source.GetData()[0] = 999.0f;
    GOAssert(
      buffer.GetData()[0] != 999.0f,
      context + ": buffer should have independent copy of data");
  }

  template <typename ManagedType>
  void TestManagedMoveConstructor(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType source(nChannels, nFrames);
    typename ManagedType::Item *originalPtr = source.GetData();

    fillWithSequential(source, 1.0f);

    ManagedType buffer(std::move(source));

    AssertMovedManagedBuffer(context, buffer, originalPtr, source, 1.0f);
  }

  template <typename ManagedType, typename ViewType>
  void TestManagedCopyAssignmentFromView(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    const unsigned nItems = nChannels * nFrames;
    std::vector<typename ManagedType::Item> sourceData(nItems);

    fillWithSequential(sourceData.data(), nItems, 10.0f);

    ViewType sourceBuffer(sourceData.data(), nChannels, nFrames);
    ManagedType buffer(1, 1); // Different size initially

    buffer = sourceBuffer;

    AssertDimensions(context, buffer, nChannels, nFrames);
    AssertSequentialData(context, buffer, 10.0f);
  }

  template <typename ManagedType>
  void TestManagedCopyAssignmentFromManaged(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType source(nChannels, nFrames);

    fillWithSequential(source, 20.0f);

    ManagedType buffer(1, 2); // Different size initially

    buffer = source;

    AssertDimensions(context, buffer, nChannels, nFrames);
    AssertSequentialData(context, buffer, 20.0f);

    source.GetData()[0] = 999.0f;
    GOAssert(
      buffer.GetData()[0] != 999.0f,
      context + ": buffer should have independent copy of data");
  }

  template <typename ManagedType>
  void TestManagedMoveAssignment(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType source(nChannels, nFrames);
    typename ManagedType::Item *originalPtr = source.GetData();

    fillWithSequential(source, 30.0f);

    ManagedType buffer(1, 1); // Different size initially

    buffer = std::move(source);

    AssertMovedManagedBuffer(context, buffer, originalPtr, source, 30.0f);
  }

  template <typename ManagedType>
  void TestManagedResize(const std::string &context) {
    const unsigned nChannels = 2;
    const unsigned nFrames = 3;
    ManagedType buffer(nChannels, nFrames);

    fillWithSequential(buffer, 1.0f);

    typename ManagedType::Item *oldData = buffer.GetData();

    buffer.Resize(2, 5);
    AssertDimensions(context + " to larger", buffer, 2, 5);
    GOAssert(
      buffer.GetData() != oldData,
      context + ": data pointer should change after resize to larger");

    oldData = buffer.GetData();
    buffer.Resize(1, 2);
    AssertDimensions(context + " to smaller", buffer, 1, 2);
    GOAssert(
      buffer.GetData() != oldData,
      context
        + ": data pointer should change after resize to a different "
          "channel count");

    buffer.Resize(0, 0);
    AssertDimensions(context + " to zero", buffer, 0, 0);
  }

  template <typename ManagedType>
  void TestManagedResizeNoReallocation(const std::string &context) {
    ManagedType buffer(2, 3);

    fillWithSequential(buffer, 1.0f);

    typename ManagedType::Item *oldData = buffer.GetData();

    // Resize to 3x2 (same total items: 2*3 = 3*2)
    buffer.Resize(3, 2);
    GOAssert(
      buffer.GetData() == oldData,
      context
        + ": data pointer should NOT change when total items are "
          "unchanged");
    AssertDimensions(context, buffer, 3, 2);
    AssertSequentialData(context, buffer, 1.0f);
  }

  template <typename ManagedType>
  void TestManagedSwap(const std::string &context) {
    const unsigned nChannels1 = 2;
    const unsigned nFrames1 = 3;
    const unsigned nChannels2 = 3;
    const unsigned nFrames2 = 2;

    ManagedType buffer1(nChannels1, nFrames1);
    ManagedType buffer2(nChannels2, nFrames2);
    typename ManagedType::Item *data1 = buffer1.GetData();
    typename ManagedType::Item *data2 = buffer2.GetData();

    fillWithSequential(buffer1, 100.0f);
    fillWithSequential(buffer2, 200.0f);

    swap(buffer1, buffer2);

    AssertDimensions(context + " buffer1", buffer1, nChannels2, nFrames2);
    AssertDimensions(context + " buffer2", buffer2, nChannels1, nFrames1);
    GOAssert(
      buffer1.GetData() == data2,
      context + ": buffer1 should have buffer2's data after swap");
    GOAssert(
      buffer2.GetData() == data1,
      context + ": buffer2 should have buffer1's data after swap");
    AssertSequentialData(context + " buffer1", buffer1, 200.0f);
    AssertSequentialData(context + " buffer2", buffer2, 100.0f);
  }

private:
  /** Shared assertion body for TestManagedMoveConstructor() and
   *  TestManagedMoveAssignment(). */
  template <typename ManagedType>
  void AssertMovedManagedBuffer(
    const std::string &context,
    const ManagedType &buffer,
    const typename ManagedType::Item *originalPtr,
    const ManagedType &source,
    float offset) {
    GOAssert(buffer.isValid(), context + ": moved-to buffer should be valid");
    GOAssert(
      buffer.GetData() == originalPtr,
      context + ": moved-to buffer should have same data pointer");

    AssertSequentialData(context, buffer, offset);

    GOAssert(
      !source.isValid(), context + ": moved-from buffer should be invalid");
    GOAssert(
      !source.GetData(),
      context + ": moved-from buffer should have null data pointer");
  }
};

#endif /* GOTESTSOUNDBUFFERBASE_H */
