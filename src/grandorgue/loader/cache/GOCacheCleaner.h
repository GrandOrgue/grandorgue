/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOCACHECLEANER_H
#define GOCACHECLEANER_H

class GOConfig;
class wxArrayString;

class GOCacheCleaner {
private:
  GOConfig &m_config;

  wxArrayString GetOrganIDList();
  wxArrayString GetArchiveIDList();

public:
  GOCacheCleaner(GOConfig &settings);
  ~GOCacheCleaner();

  void Cleanup();
};

#endif
