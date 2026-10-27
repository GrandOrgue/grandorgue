/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_COMPILER_H
#define GO_COMPILER_H

/**
 * Forces the compiler to inline a function even at -Og (the Debug build),
 * where it otherwise would not. Use sparingly, only for tiny hot-path
 * functions. It includes the inline specifier, so use it instead of it.
 */
#if defined(_MSC_VER)
#define GO_FORCE_INLINE __forceinline
#else
#define GO_FORCE_INLINE inline __attribute__((always_inline))
#endif

#endif /* GO_COMPILER_H */
