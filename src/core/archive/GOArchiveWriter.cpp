/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "GOArchiveWriter.h"

#include <zlib.h>

GOArchiveWriter::GOArchiveWriter()
  : m_Offset(0), m_directory(0), m_Entries(0), m_Names() {}

GOArchiveWriter::~GOArchiveWriter() { Close(); }

bool GOArchiveWriter::Open(wxString filename) {
  m_Offset = 0;
  m_directory.resize(0);
  m_Entries = 0;
  m_Names.resize(0);
  return m_File.Open(filename, wxFile::write);
}

bool GOArchiveWriter::Write(const void *data, size_t size) {
  m_Offset += size;
  return m_File.Write(data, size) == size;
}

bool GOArchiveWriter::Add(wxString name, const GOBuffer<uint8_t> &content) {
  name.Replace(wxT("\\"), wxT("/"));
  for (unsigned i = 0; i < m_Names.size(); i++)
    if (m_Names[i] == name)
      return false;
  m_Names.push_back(name);
  wxCharBuffer fname = name.utf8_str();
  GOZipLocalHeader sig;
  sig.signature = ZIP_LOCAL_HEADER;
  sig.version_extract = ZIP_ZIP64_VERSION;
  sig.flags = 0x0800;
  sig.compression = 0;
  sig.modification_time = 0;
  sig.modification_date = 0;
  sig.crc = crc32(crc32(0, Z_NULL, 0), content.get(), content.GetSize());
  sig.compressed_size = 0xffffffff;
  sig.uncompressed_size = 0xffffffff;
  sig.name_length = fname.length();
  sig.extra_length = sizeof(GOZipLocal64ExtendHeader);
  GOZipLocal64ExtendHeader locext;
  locext.header.type = 0x0001;
  locext.header.size = sizeof(locext) - sizeof(locext.header);
  locext.uncompressed_size = content.GetSize();
  locext.compressed_size = content.GetSize();

  GOZipCentralHeader csig;
  csig.signature = ZIP_CENTRAL_DIRECTORY_HEADER;
  csig.version_creator = ZIP_ZIP64_VERSION;
  csig.version_extract = sig.version_extract;
  csig.flags = sig.flags;
  csig.compression = sig.compression;
  csig.modification_time = sig.modification_time;
  csig.modification_date = sig.modification_date;
  csig.crc = sig.crc;
  csig.compressed_size = sig.compressed_size;
  csig.uncompressed_size = sig.uncompressed_size;
  csig.name_length = sig.name_length;
  csig.extra_length = sizeof(GOZipCentral64ExtendHeader);
  csig.comment_length = 0;
  csig.disk_number = 0;
  csig.internal_attributes = 0;
  csig.external_attributes = 0;
  csig.offset = 0xffffffff;
  GOZipCentral64ExtendHeader centralext;
  centralext.header.type = 0x0001;
  centralext.header.size = sizeof(centralext) - sizeof(centralext.header);
  centralext.uncompressed_size = locext.uncompressed_size;
  centralext.compressed_size = locext.compressed_size;
  centralext.offset = m_Offset;

  if (!Write(&sig, sizeof(sig)))
    return false;
  if (!Write(fname.data(), fname.length()))
    return false;
  if (!Write(&locext, sizeof(locext)))
    return false;
  if (!Write(content.get(), content.GetSize()))
    return false;

  GOBuffer<uint8_t> entry;
  entry.Append((const uint8_t *)&csig, sizeof(csig));
  entry.Append((const uint8_t *)fname.data(), fname.length());
  entry.Append((const uint8_t *)&centralext, sizeof(centralext));

  m_directory.Append(entry);

  m_Entries++;

  return true;
}

bool GOArchiveWriter::Close() {
  if (!m_File.IsOpened())
    return false;

  GOZipEnd64Record end64;
  end64.signature = 0x06064b50;
  end64.size = sizeof(end64) - 12;
  end64.version_creator = ZIP_ZIP64_VERSION;
  end64.version_extract = ZIP_ZIP64_VERSION;
  end64.current_disk = 0;
  end64.directory_disk = 0;
  end64.entry_count_disk = m_Entries;
  end64.entry_count = m_Entries;
  end64.directory_size = m_directory.GetSize();
  end64.directory_offset = m_Offset;

  if (!Write(m_directory.get(), m_directory.GetSize()))
    return false;

  GOZipEnd64Locator locator64;
  locator64.signature = 0x07064b50;
  locator64.end_record_disk = 0;
  locator64.end_record_offset = m_Offset;
  locator64.disk_count = 1;

  GOZipEndRecord end;
  end.signature = ZIP_END_RECORD;
  end.current_disk = 0;
  end.directory_disk = 0;
  end.entry_count_disk = 0xffff;
  end.entry_count = 0xffff;
  end.directory_size = 0xffffffff;
  end.directory_offset = 0xffffffff;
  end.comment_len = 0;

  if (!Write(&end64, sizeof(end64)))
    return false;
  if (!Write(&locator64, sizeof(locator64)))
    return false;
  if (!Write(&end, sizeof(end)))
    return false;

  m_File.Close();
  return true;
}
