/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOOPENEDFILE_H
#define GOOPENEDFILE_H

#include <wx/string.h>

template <class T> class GOBuffer;

class GOOpenedFile {
public:
  virtual ~GOOpenedFile() {}

  virtual size_t GetSize() = 0;
  virtual const wxString GetName() = 0;
  virtual const wxString GetPath() = 0;

  virtual bool Open() = 0;
  virtual void Close() = 0;
  virtual size_t Read(void *buffer, size_t len) = 0;

  template <class T> bool Read(GOBuffer<T> &buf) {
    return Read(buf.get(), buf.GetSize()) == buf.GetSize();
  }

  template <class T> bool ReadContent(GOBuffer<T> &buf) {
    if (!Open())
      return false;
    if (GetSize() % sizeof(T)) {
      Close();
      return false;
    }
    buf.resize(GetSize() / sizeof(T));
    bool ret = Read(buf);
    Close();
    return ret;
  }

  virtual bool isValid() { return true; }
};

#endif
