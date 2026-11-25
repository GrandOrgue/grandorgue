/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOFLOAT32_H
#define GOFLOAT32_H

#include "GOUnaligned.h"

/** A 32-bit IEEE float that can be stored at any address */
using GOFloat32 = GOUnaligned<float>;

static_assert(alignof(GOFloat32) == 1 && sizeof(GOFloat32) == 4);

#endif /* GOFLOAT32_H */
