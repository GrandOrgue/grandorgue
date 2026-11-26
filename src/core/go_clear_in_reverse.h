/*
 * Copyright 2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GO_CLEAR_IN_REVERSE_H
#define GO_CLEAR_IN_REVERSE_H

/**
 * Deletes the elements of the container one by one from the end until
 * newSize elements remain, so that they are destroyed in the reverse order of
 * adding. The order of ptr_vector::clear(), resize() and of the destructor is
 * defined (from the beginning), but it is unspecified for
 * std::vector<std::unique_ptr<T>>. This function works the same for both.
 *
 * It is used where the owned objects refer to each other (the organ model and
 * the controller). The nested containers (the pipes of a rank, the stops of a
 * manual, the combinations of the setter) are not cleared this way: their
 * elements are independent in destructors.
 * @tparam C a container of owning pointers with size() and pop_back()
 *   (ptr_vector or std::vector<std::unique_ptr<T>>)
 * @param container the container to shrink
 * @param newSize the number of the elements to keep
 */
template <class C>
void go_clear_in_reverse(C &container, unsigned newSize = 0) {
  while (container.size() > newSize)
    container.pop_back();
}

#endif /* GO_CLEAR_IN_REVERSE_H */
