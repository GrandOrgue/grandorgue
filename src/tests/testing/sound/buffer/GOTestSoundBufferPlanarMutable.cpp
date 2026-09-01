/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2024-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOTestSoundBufferPlanarMutable.h"

#include <cstring>
#include <format>
#include <vector>

#include "sound/buffer/GOSoundBufferMutable.h"
#include "sound/buffer/GOSoundBufferPlanarMutable.h"

const std::string GOTestSoundBufferPlanarMutable::TEST_NAME
  = "GOTestSoundBufferPlanarMutable";

void GOTestSoundBufferPlanarMutable::TestGetChannelBufferWriteThrough() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, nChannels, nFrames)

  buffer.FillWithSilence();
  buffer.GetChannelBuffer(1).GetData()[2] = 42.0f;

  for (unsigned itemI = 0, n = buffer.GetNItems(); itemI < n; ++itemI)
    AssertItemEqual(
      "GetChannelBufferWriteThrough",
      itemI,
      itemI == 1 * nFrames + 2 ? 42.0f : 0.0f,
      buffer.GetData()[itemI]);
}

void GOTestSoundBufferPlanarMutable::TestFillWithSilence() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(buffer, nChannels, nFrames)

  fillWithSequential(buffer, 1.0f);
  buffer.FillWithSilence();

  for (unsigned itemI = 0, n = buffer.GetNItems(); itemI < n; ++itemI)
    AssertItemEqual("FillWithSilence", itemI, 0.0f, buffer.GetData()[itemI]);
}

void GOTestSoundBufferPlanarMutable::TestCopyFrom() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(srcBuffer, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(dstBuffer, nChannels, nFrames)

  fillWithSequential(srcBuffer, 1.0f);
  dstBuffer.FillWithSilence();

  dstBuffer.CopyFrom(srcBuffer);

  AssertSequentialData("CopyFrom", dstBuffer, 1.0f);
}

void GOTestSoundBufferPlanarMutable::TestAddFrom() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferA, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferB, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(result, nChannels, nFrames)

  fillWithSequential(bufferA, 1.0f);
  fillWithSequential(bufferB, 10.0f);

  result.CopyFrom(bufferA);
  result.AddFrom(bufferB);

  for (unsigned itemI = 0, n = result.GetNItems(); itemI < n; ++itemI)
    AssertItemEqual(
      "AddFrom",
      itemI,
      bufferA.GetData()[itemI] + bufferB.GetData()[itemI],
      result.GetData()[itemI]);
}

void GOTestSoundBufferPlanarMutable::TestAddFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferA, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(bufferB, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(result, nChannels, nFrames)

  fillWithSequential(bufferA, 1.0f);
  fillWithSequential(bufferB, 10.0f);

  const float coefficient = 2.0f;

  result.CopyFrom(bufferA);
  result.AddFrom(bufferB, coefficient);

  for (unsigned itemI = 0, n = result.GetNItems(); itemI < n; ++itemI)
    AssertItemEqual(
      "AddFrom+coeff",
      itemI,
      bufferA.GetData()[itemI] + bufferB.GetData()[itemI] * coefficient,
      result.GetData()[itemI]);
}

void GOTestSoundBufferPlanarMutable::TestCopyChannelFrom() {
  const unsigned srcNChannels = 3;
  const unsigned dstNChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(srcBuffer, srcNChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(dstBuffer, dstNChannels, nFrames)

  fillWithSequential(srcBuffer, 0.0f);
  dstBuffer.FillWithSilence();

  // Copy channel 1 of src into channel 0 of dst
  dstBuffer.CopyChannelFrom(srcBuffer, 1, 0);

  AssertSequentialData(
    "CopyChannelFrom",
    dstBuffer.GetChannelBuffer(0),
    static_cast<float>(1 * nFrames));

  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    AssertItemEqual(
      "CopyChannelFrom ch1 untouched",
      frameI,
      0.0f,
      dstBuffer.GetChannelBuffer(1).GetData()[frameI]);
}

void GOTestSoundBufferPlanarMutable::TestAddChannelFrom() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(srcBuffer, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(dstBuffer, nChannels, nFrames)

  fillWithSequential(srcBuffer, 1.0f);
  fillWithSequential(dstBuffer, 100.0f);

  dstBuffer.AddChannelFrom(srcBuffer, 0, 1);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    AssertItemEqual(
      "AddChannelFrom ch0 untouched",
      frameI,
      100.0f + static_cast<float>(frameI),
      dstBuffer.GetChannelBuffer(0).GetData()[frameI]);
    AssertItemEqual(
      "AddChannelFrom ch0->ch1",
      frameI,
      (100.0f + static_cast<float>(nFrames + frameI))
        + srcBuffer.GetChannelBuffer(0).GetData()[frameI],
      dstBuffer.GetChannelBuffer(1).GetData()[frameI]);
  }
}

void GOTestSoundBufferPlanarMutable::TestAddChannelFromWithCoefficient() {
  const unsigned nChannels = 2;
  const unsigned nFrames = 3;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(srcBuffer, nChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(dstBuffer, nChannels, nFrames)

  fillWithSequential(srcBuffer, 1.0f);
  fillWithSequential(dstBuffer, 100.0f);

  const float coefficient = 0.5f;

  dstBuffer.AddChannelFrom(srcBuffer, 0, 1, coefficient);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    const float expectedCh1 = (100.0f + static_cast<float>(nFrames + frameI))
      + srcBuffer.GetChannelBuffer(0).GetData()[frameI] * coefficient;

    AssertItemEqual(
      "AddChannelFrom+coeff",
      frameI,
      dstBuffer.GetChannelBuffer(1).GetData()[frameI],
      expectedCh1);
  }
}

void GOTestSoundBufferPlanarMutable::TestDeinterleaveFrom() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  // interleavedData[f * nChannels + c] = c * 100 + f
  std::vector<GOSoundBuffer::Item> interleavedData(nChannels * nFrames);
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    for (unsigned channelI = 0; channelI < nChannels; ++channelI)
      interleavedData[frameI * nChannels + channelI]
        = static_cast<float>(channelI * 100 + frameI);

  GOSoundBufferMutable interleavedBuffer(
    interleavedData.data(), nChannels, nFrames);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(planarBuffer, nChannels, nFrames)
  planarBuffer.FillWithSilence();

  planarBuffer.DeinterleaveFrom(interleavedBuffer);

  for (unsigned channelI = 0; channelI < nChannels; ++channelI)
    for (unsigned frameI = 0; frameI < nFrames; ++frameI)
      AssertItemEqual(
        "DeinterleaveFrom",
        channelI * nFrames + frameI,
        static_cast<float>(channelI * 100 + frameI),
        planarBuffer.GetData()[channelI * nFrames + frameI]);
}

void GOTestSoundBufferPlanarMutable::TestAddDeinterleavedFrom() {
  const unsigned nChannels = 3;
  const unsigned nFrames = 4;

  // interleavedData[f * nChannels + c] = c * 100 + f
  std::vector<GOSoundBuffer::Item> interleavedData(nChannels * nFrames);
  for (unsigned frameI = 0; frameI < nFrames; ++frameI)
    for (unsigned channelI = 0; channelI < nChannels; ++channelI)
      interleavedData[frameI * nChannels + channelI]
        = static_cast<float>(channelI * 100 + frameI);

  GOSoundBufferMutable interleavedBuffer(
    interleavedData.data(), nChannels, nFrames);

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(planarBuffer, nChannels, nFrames)
  fillWithSequential(planarBuffer, 1000.0f);

  std::vector<GOSoundBuffer::Item> oldData(nChannels * nFrames);
  std::memcpy(oldData.data(), planarBuffer.GetData(), planarBuffer.GetNBytes());

  planarBuffer.AddDeinterleavedFrom(interleavedBuffer);

  for (unsigned channelI = 0; channelI < nChannels; ++channelI)
    for (unsigned frameI = 0; frameI < nFrames; ++frameI)
      AssertItemEqual(
        "AddDeinterleavedFrom",
        channelI * nFrames + frameI,
        oldData[channelI * nFrames + frameI]
          + static_cast<float>(channelI * 100 + frameI),
        planarBuffer.GetData()[channelI * nFrames + frameI]);
}

void GOTestSoundBufferPlanarMutable::TestSingleChannelBuffer() {
  const unsigned monoNChannels = 1;
  const unsigned stereoNChannels = 2;
  const unsigned nFrames = 4;

  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(monoBuffer, monoNChannels, nFrames)
  GO_DECLARE_LOCAL_SOUND_BUFFER_PLANAR(stereoBuffer, stereoNChannels, nFrames)

  fillWithSequential(monoBuffer, 1.0f);
  stereoBuffer.FillWithSilence();

  stereoBuffer.CopyChannelFrom(monoBuffer, 0, 0);
  stereoBuffer.AddChannelFrom(monoBuffer, 0, 1, 0.5f);

  for (unsigned frameI = 0; frameI < nFrames; ++frameI) {
    AssertItemEqual(
      "SingleChannel ch0",
      frameI,
      monoBuffer.GetData()[frameI],
      stereoBuffer.GetChannelBuffer(0).GetData()[frameI]);
    AssertItemEqual(
      "SingleChannel ch1",
      frameI,
      monoBuffer.GetData()[frameI] * 0.5f,
      stereoBuffer.GetChannelBuffer(1).GetData()[frameI]);
  }
}

void GOTestSoundBufferPlanarMutable::run() {
  TestGetChannelBufferWriteThrough();
  TestFillWithSilence();
  TestCopyFrom();
  TestAddFrom();
  TestAddFromWithCoefficient();
  TestCopyChannelFrom();
  TestAddChannelFrom();
  TestAddChannelFromWithCoefficient();
  TestDeinterleaveFrom();
  TestAddDeinterleavedFrom();
  TestSingleChannelBuffer();
}
