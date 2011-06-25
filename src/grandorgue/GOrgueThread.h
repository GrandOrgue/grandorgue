/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef GORGUETHREAD_H
#define GORGUETHREAD_H

class GOrgueThread : public wxThread
{
public:
    GOrgueThread() : wxThread(wxTHREAD_JOINABLE)
    {
    }

    void Go(int file, char* buffer, unsigned length)
    {
        m_file = file;
        m_buffer = buffer;
        m_length = length;
    }

    void* Entry()
    {
	  unsigned remain = m_length;
	  for(;;)
		{
#ifdef _WIN32
		  remain -= _read(m_file, m_buffer, remain >= BUFSIZ ? BUFSIZ : remain);
#endif
#ifdef linux
		  remain -= read(m_file, m_buffer, remain >= BUFSIZ ? BUFSIZ : remain); // TODO better error handling
#endif
		  if (!remain || TestDestroy())
			break;
		  m_buffer += BUFSIZ;
		  Yield();
		}
#ifdef _WIN32
	  _close(m_file);
#endif
#ifdef linux
	  close(m_file);
#endif
	  return (void*)m_length;
    };

    int m_file;
    char* m_buffer;
    unsigned m_length;
};

#endif
