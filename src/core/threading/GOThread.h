/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOTHREAD_H
#define GOTHREAD_H

#include <atomic>
#include <thread>

class GOThread {
private:
  std::thread m_Thread;
  std::atomic_bool m_Stop;

  static void EntryPoint(GOThread *thread);

protected:
  virtual void Entry() = 0;

public:
  GOThread();
  virtual ~GOThread();

  void Start();
  void MarkForStop();
  void Wait();
  void Stop();

  bool ShouldStop();
};

#endif
