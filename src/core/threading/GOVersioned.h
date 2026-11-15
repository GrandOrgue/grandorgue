/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOVERSIONED_H
#define GOVERSIONED_H

#include <atomic>
#include <cstring>
#include <type_traits>

/**
 * A value of type T that one thread publishes and other threads read without
 * locks. A reader keeps its own Snapshot and renews it by RenewSnapshot().
 * A snapshot is renewed only when a newer value has been published and is not
 * being written at this moment, so a reader never sees a mix of the old and
 * new value and never waits for the writer.
 *
 * T must be default constructible and trivially copyable. The published value
 * is stored as an array of atomic words that are accessed with relaxed
 * atomic operations, so a reader may copy the words while the writer is
 * storing them without a data race; a copy that overlapped with a publication
 * is torn, but it is discarded by the re-validation in RenewSnapshot() before
 * it is used.
 */
template <typename T> class GOVersioned {
  static_assert(
    std::is_trivially_copyable_v<T>,
    "GOVersioned requires a trivially copyable T");
  static_assert(
    std::atomic<unsigned>::is_always_lock_free,
    "GOVersioned requires lock-free atomics");

private:
  static constexpr unsigned N_WORDS
    = (sizeof(T) + sizeof(unsigned) - 1) / sizeof(unsigned);

  /** The words of the published value. Accessed only by relaxed atomic
   * operations */
  std::atomic<unsigned> m_ValueWords[N_WORDS]{};
  /** Increased by every Publish(). 0 means that nothing has been published */
  std::atomic<unsigned> m_version{0};
  /** True while Publish() is replacing m_ValueWords: the intermediate
   * inconsistent state that readers must not copy from */
  std::atomic<bool> m_IsMutatingNow{false};

public:
  /** The reader's consistent copy of a published value. Not shared between
   * threads: every reader owns its own snapshot. Filled by RenewSnapshot() */
  class Snapshot {
    friend class GOVersioned;

  private:
    T m_value{};
    unsigned m_version = 0; // 0 means that nothing has been received yet

  public:
    /** @return the value copied by the last successful RenewSnapshot(); a
     * default-constructed T until then */
    const T &Get() const { return m_value; }
  };

  /**
   * Publishes a new value. Must not be called concurrently with another
   * Publish() of the same object (the writers are to be serialized by the
   * caller). Never blocks readers.
   * @param newValue the value that readers will receive
   */
  void Publish(const T &newValue) {
    unsigned newWords[N_WORDS] = {};
    const unsigned *pNewWord = newWords;
    std::atomic<unsigned> *pValueWord = m_ValueWords;

    std::memcpy(newWords, &newValue, sizeof(T));
    m_IsMutatingNow.store(true, std::memory_order_relaxed);
    // the flag must become visible before any write of the new value
    std::atomic_thread_fence(std::memory_order_release);
    for (unsigned nWordsLeft = N_WORDS; nWordsLeft;
         nWordsLeft--, pNewWord++, pValueWord++)
      pValueWord->store(*pNewWord, std::memory_order_relaxed);
    m_version.fetch_add(1, std::memory_order_release);
    m_IsMutatingNow.store(false, std::memory_order_release);
  }

  /**
   * Renews the snapshot if a newer consistent value has been published.
   * Idempotent and wait-free: copies the value only when the published
   * version differs from the version of the snapshot.
   * @param ioSnapshot the reader's snapshot to renew
   * @return true if the snapshot was renewed; false if there is no newer
   *   value or it is being published right now (the snapshot is left
   *   unchanged and the next call will try again)
   */
  bool RenewSnapshot(Snapshot &ioSnapshot) const {
    // acquire: the value of this version is visible after the load
    const unsigned newVersion = m_version.load(std::memory_order_acquire);
    bool isRenewed = false;

    // the flag check is only an optimization; the correctness is ensured by
    // the re-validation below
    if (
      newVersion != ioSnapshot.m_version
      && !m_IsMutatingNow.load(std::memory_order_relaxed)) {
      unsigned newWords[N_WORDS];
      unsigned *pNewWord = newWords;
      const std::atomic<unsigned> *pValueWord = m_ValueWords;

      for (unsigned nWordsLeft = N_WORDS; nWordsLeft;
           nWordsLeft--, pNewWord++, pValueWord++)
        *pNewWord = pValueWord->load(std::memory_order_relaxed);

      // the flag check below must not be moved before the copy
      std::atomic_thread_fence(std::memory_order_acquire);
      // Re-validate: the writer may have started or finished while copying.
      // acquire: if the flag was reset by the publication that tore the copy,
      // its new version is visible to the next load
      if (
        !m_IsMutatingNow.load(std::memory_order_acquire)
        && m_version.load(std::memory_order_relaxed) == newVersion) {
        std::memcpy(&ioSnapshot.m_value, newWords, sizeof(T));
        ioSnapshot.m_version = newVersion;
        isRenewed = true;
      }
    }
    return isRenewed;
  }
};

#endif /* GOVERSIONED_H */
