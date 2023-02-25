/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2023 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef PTRVECTOR_H
#define PTRVECTOR_H

#include <vector>

template <class T> class ptr_vector : protected std::vector<T *> {
private:
  /* Disable copying as it will break things */
  ptr_vector(const ptr_vector &);
  const ptr_vector operator=(const ptr_vector &);

public:
  ptr_vector(unsigned new_size = 0) : std::vector<T *>(new_size) {
    for (unsigned i = 0; i < new_size; i++)
      at(i) = 0;
  }

  ~ptr_vector() {
    for (unsigned i = 0; i < size(); i++)
      if (at(i))
        delete at(i);
  }

  T *&operator[](unsigned pos) { return at(pos); }

  const T *operator[](unsigned pos) const { return at(pos); }

  T *&at(unsigned pos) { return std::vector<T *>::at(pos); }

  const T *at(unsigned pos) const { return std::vector<T *>::at(pos); }

  unsigned size() const { return std::vector<T *>::size(); }

  void clear() { resize(0); }

  void resize(unsigned new_size) {
    unsigned oldsize = size();
    for (unsigned i = new_size; i < oldsize; i++)
      if (at(i))
        delete at(i);
    std::vector<T *>::resize(new_size);
    for (unsigned i = oldsize; i < new_size; i++)
      at(i) = 0;
  }

  void push_back(T *ptr) { std::vector<T *>::push_back(ptr); }

  void insert(unsigned pos, T *ptr) {
    std::vector<T *>::insert(std::vector<T *>::begin() + pos, ptr);
  }

  void erase(unsigned pos) {
    if (at(pos))
      delete at(pos);
    std::vector<T *>::erase(std::vector<T *>::begin() + pos);
  }

  typename std::vector<T *>::const_iterator begin() const noexcept {
    return std::vector<T *>::begin();
  }

  typename std::vector<T *>::const_iterator end() const noexcept {
    return std::vector<T *>::end();
  }

  typename std::vector<T *>::iterator begin() noexcept {
    return std::vector<T *>::begin();
  }

  typename std::vector<T *>::iterator end() noexcept {
    return std::vector<T *>::end();
  }
};

#endif
