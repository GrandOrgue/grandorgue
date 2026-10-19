/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOPARAMRANGE_H
#define GOPARAMRANGE_H

/**
 * The allowed range of a parameter, both ends included.
 * @tparam T the type of the parameter: int or float
 */
template <typename T> struct GOParamRange {
  T m_min;
  T m_max;

  /**
   * @tparam U the type of the checked value; a double is compared as is, so a
   *   NaN is never contained
   * @param value the value to check
   * @return whether the value is within the range
   */
  template <typename U> bool Contains(U value) const {
    return value >= m_min && value <= m_max;
  }

  /**
   * @param value the value to limit
   * @return the value limited to the range; a NaN is returned as is
   */
  T Clamp(T value) const {
    return value < m_min ? m_min : (value > m_max ? m_max : value);
  }
};

#endif /* GOPARAMRANGE_H */
