/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef UPDATER_SETTINGS_H
#define UPDATER_SETTINGS_H

#include "go_defs.h"

const long TIMEOUT_MS = 5000;
const char *const USER_AGENT_HEADER = "User-Agent: GrandOrgue";
const char *const RELEASES_API_URL
  = "https://api.github.com/repos/" GITHUB_PROJECT "/releases?per_page=100";
const char *const DOWNLOAD_URL
  = "https://github.com/" GITHUB_PROJECT "/releases";

#endif