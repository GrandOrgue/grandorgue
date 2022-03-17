/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2022 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#include "threading_impl.h"

#if defined GO_STD_MUTEX
#include <chrono>
std::chrono::milliseconds const THREADING_WAIT_TIMEOUT(WAIT_TIMEOUT_MS);
#endif
