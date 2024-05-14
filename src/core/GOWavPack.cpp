/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOWavPack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GOAlloc.h"

GOWavPack::GOWavPack(const GOBuffer<uint8_t> &file)
  : m_data(file),
    m_Samples(),
    m_Wrapper(),
    m_pos(0),
    m_OrigDataLen(0),
    m_context(0) {}

GOWavPack::~GOWavPack() {
  m_Samples.free();
  m_Wrapper.free();
  if (m_context)
    WavpackCloseFile(m_context);
}

bool GOWavPack::IsWavPack(const GOBuffer<uint8_t> &data) {
  return data.GetSize() > 10 && !memcmp(data.get(), "wvpk", 4);
}

GOBuffer<uint8_t> GOWavPack::GetSamples() { return std::move(m_Samples); }

GOBuffer<uint8_t> GOWavPack::GetWrapper() { return std::move(m_Wrapper); }

unsigned GOWavPack::GetOrigDataLen() { return m_OrigDataLen; }

bool GOWavPack::Unpack() {
  m_context
    = WavpackOpenFileInputEx(&m_Reader, this, NULL, NULL, OPEN_WRAPPER, 0);
  if (!m_context)
    return false;

  unsigned header = WavpackGetWrapperBytes(m_context);
  if (!header)
    return false;

  unsigned channels = WavpackGetNumChannels(m_context);
  unsigned samples = WavpackGetNumSamples(m_context);
  m_Samples.resize(channels * samples * 4);
  unsigned res
    = WavpackUnpackSamples(m_context, (int32_t *)m_Samples.get(), samples);
  if (res != samples)
    return false;

  m_OrigDataLen = channels * samples * WavpackGetBytesPerSample(m_context);

  WavpackSeekTrailingWrapper(m_context);
  unsigned trailer = WavpackGetWrapperBytes(m_context) - header;

  m_Wrapper.free();
  m_Wrapper.Append(WavpackGetWrapperData(m_context), header + trailer);
  WavpackFreeWrapper(m_context);

  WavpackCloseFile(m_context);
  m_context = NULL;

  return true;
}

uint32_t GOWavPack::GetLength(void *id) {
  return ((GOWavPack *)id)->GetLength();
}

int32_t GOWavPack::ReadBytes(void *id, void *data, int32_t bcount) {
  return ((GOWavPack *)id)->ReadBytes(data, bcount);
}

int GOWavPack::PushBackByte(void *id, int c) {
  return ((GOWavPack *)id)->PushBackByte(c);
}

uint32_t GOWavPack::GetPos(void *id) { return ((GOWavPack *)id)->GetPos(); }

int GOWavPack::CanSeek(void *id) { return ((GOWavPack *)id)->CanSeek(); }

int GOWavPack::SetPosAbs(void *id, uint32_t pos) {
  return ((GOWavPack *)id)->SetPosAbs(pos);
}

int GOWavPack::SetPosRel(void *id, int32_t delta, int mode) {
  return ((GOWavPack *)id)->SetPosRel(delta, mode);
}

uint32_t GOWavPack::GetLength() { return m_data.GetSize(); }

int32_t GOWavPack::ReadBytes(void *data, int32_t bcount) {
  if (m_pos + bcount > m_data.GetSize())
    bcount = m_data.GetSize() - m_pos;
  memcpy(data, m_data.get() + m_pos, bcount);
  m_pos += bcount;
  return bcount;
}

int GOWavPack::PushBackByte(int c) {
  if (m_pos > 0 && m_pos < m_data.GetSize() && m_data[m_pos - 1] == c) {
    m_pos--;
    return c;
  }
  return EOF;
}

uint32_t GOWavPack::GetPos() { return m_pos; }

int GOWavPack::CanSeek() { return 1; }

int GOWavPack::SetPosAbs(uint32_t pos) {
  if (pos < m_data.GetCount()) {
    m_pos = pos;
    return 0;
  }
  return -1;
}

int GOWavPack::SetPosRel(int32_t delta, int mode) {
  switch (mode) {
  case SEEK_SET:
    return SetPosAbs(delta);
  case SEEK_CUR:
    return SetPosAbs(m_pos + delta);
  case SEEK_END:
    return SetPosAbs(m_data.GetCount() + delta);
  default:
    return -1;
  }
}

WavpackStreamReader GOWavPack::m_Reader = {
  .read_bytes = ReadBytes,
  .get_pos = GetPos,
  .set_pos_abs = SetPosAbs,
  .set_pos_rel = SetPosRel,
  .push_back_byte = PushBackByte,
  .get_length = GetLength,
  .can_seek = CanSeek,
  .write_bytes = NULL};
