/*
 * Copyright 2024 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef UPDATER_SETTINGS_H
#define UPDATER_SETTINGS_H

// Don't use long timeouts here since we wait for completion on application
// exit. Also users may become annoyed if 'New release!' dialog appears while
// they are playing.
const long TIMEOUT_MS = 5000;
const char *const USER_AGENT_HEADER = "User-Agent: GrandOrgue";
const char *const LATEST_RELEASE_API_URL
  = "https://api.github.com/repos/GrandOrgue/GrandOrgue/releases/latest";
const char *const DOWNLOAD_URL
  = "https://github.com/GrandOrgue/grandorgue/releases";

#endif