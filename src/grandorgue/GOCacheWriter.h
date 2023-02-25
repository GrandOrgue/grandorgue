/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCACHEWRITER_H_
#define GOCACHEWRITER_H_

class wxOutputStream;

class GOCacheWriter {
  wxOutputStream *m_zstream;
  wxOutputStream *m_stream;

public:
  GOCacheWriter(wxOutputStream &stream, bool compressed);
  virtual ~GOCacheWriter();

  bool WriteHeader();
  bool Write(const void *data, unsigned length);
  /* Write an bigger malloced block */
  bool WriteBlock(const void *data, unsigned length);

  void Close();
};

#endif
