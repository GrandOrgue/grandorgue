/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2025 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "config/GOConfigFileWriter.h"

#include <wx/file.h>
#include <wx/intl.h>
#include <wx/log.h>

#include "GOBuffer.h"
#include "GOCompress.h"

GOConfigFileWriter::GOConfigFileWriter() : m_Entries() {}

void GOConfigFileWriter::AddEntry(
  wxString group, wxString name, wxString value) {
  std::map<wxString, wxString> &g = m_Entries[group];
  assert(g.find(name) == g.end());
  g[name] = value;
}

bool GOConfigFileWriter::GetFileContent(
  GOBuffer<uint8_t> &buf, bool isToCompress) {
  uint8_t bom[] = {0xEF, 0xBB, 0xBF};
  wxString content = wxEmptyString;

  for (std::map<wxString, std::map<wxString, wxString>>::const_iterator i
       = m_Entries.begin();
       i != m_Entries.end();
       i++) {
    const std::map<wxString, wxString> &g = i->second;
    const wxString group = i->first;
    wxString groupContent = wxT("[") + group + wxT("]\n");

    for (std::map<wxString, wxString>::const_iterator j = g.begin();
         j != g.end();
         j++) {
      const wxString &name = j->first;
      const wxString &value = j->second;
      groupContent += name + wxT("=") + value + wxT("\n");
    }
    content += groupContent;
  }
  wxCharBuffer b = content.utf8_str();
  buf.free();
  buf.Append(bom, sizeof(bom));
  buf.Append((const uint8_t *)b.data(), b.length());

  if (isToCompress && !compressBuffer(buf))
    return false;

  return true;
}

bool GOConfigFileWriter::Save(const wxString &fileName, bool isToCompress) {
  wxFile out;
  GOBuffer<uint8_t> buf;

  bool isOk = GetFileContent(buf, isToCompress);

  if (isOk)
    isOk = out.Create(fileName, true);

  if (isOk)
    isOk = out.Write(buf.get(), buf.GetSize());
  if (isOk)
    isOk = out.Flush();
  if (isOk)
    isOk = out.Close();
  return isOk;
}
