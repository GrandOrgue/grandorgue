/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOThread.h"

#include "atomic.h"

GOThread::GOThread() : m_Thread(), m_Stop(false) {}

GOThread::~GOThread() { Stop(); }

void GOThread::Start() {
  m_Stop = false;
  if (m_Thread.joinable())
    return;
  m_Thread = std::thread(GOThread::EntryPoint, this);
}

void GOThread::MarkForStop() { m_Stop = true; }

void GOThread::Wait() {
  if (m_Thread.joinable())
    m_Thread.join();
}

void GOThread::Stop() {
  MarkForStop();
  Wait();
}

bool GOThread::ShouldStop() { return load_once(m_Stop); }

void GOThread::EntryPoint(GOThread *thread) { thread->Entry(); }
