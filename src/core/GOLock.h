/*
* Copyright 2006 Milan Digital Audio LLC
* Copyright 2009-2019 GrandOrgue contributors (see AUTHORS)
* License GPL-2.0 or later (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
*/

#ifndef GOLOCK_H
#define GOLOCK_H

#include <wx/log.h>
#include <wx/stackwalk.h>

#define GO_PRINTCONTENTION 0

#if wxUSE_STACKWALKER
class GOStackPrinter : public wxStackWalker
{
	void* m_ptr;

	GOStackPrinter(void* p)
	{
		m_ptr = p;
	}

	void OnStackFrame(const wxStackFrame& frame)
	{
		wxLogWarning(wxT("%p: [%2d] %s(%p)\t%s:%d"), m_ptr, frame.GetLevel(), frame.GetName().c_str(), frame.GetAddress(), frame.GetFileName().c_str(), frame.GetLine());
	}

public:
	static void printStack(void *p)
	{
		GOStackPrinter print(p);
		print.Walk();
	}
};
#else
class GOStackPrinter
{
public:
	static void printStack(void *p)
	{
	}
};
#endif

#if 1
//#ifdef __WIN32__

#include <wx/thread.h>

class GOWaitQueue
{
private:
	wxSemaphore m_Wait;
public:
	GOWaitQueue()
	{
	}

	~GOWaitQueue()
	{
	}

	void Wait()
	{
		m_Wait.Wait();
	}

	void Wakeup()
	{
		m_Wait.Post();
	}
};
#else

#include <atomic>
#include <mutex>

class GOWaitQueue
{
private:
	std::mutex m_Wait;
public:
	GOWaitQueue()
	{
		m_Wait.lock();
	}

	~GOWaitQueue()
	{
		m_Wait.unlock();
	}

	void Wait()
	{
		m_Wait.lock();
	}

	void Wakeup()
	{
		m_Wait.unlock();
	}
};
#endif

#endif
