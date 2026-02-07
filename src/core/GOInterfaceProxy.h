/*
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2026 GrandOrgue contributors (see AUTHORS)
 * License GPL-2.0 or later
 * (https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
 */

#ifndef GOINTERFACEPROXY_H
#define GOINTERFACEPROXY_H

#include <utility> // std::forward

/**
 * Abstract proxy implementation for any interface.
 * The instance may be connected or not connected to a target instance of the
 * same interface. It forwards calls of the interface methods to the target
 * instance.
 *
 * This class template provides two forwarding strategies for derived classes:
 * 1. ForwardCall() - safe, does nothing if proxy is not connected (void methods
 * only)
 * 2. ForwardCall(defaultValue) - safe, returns default value if not connected
 *
 * @tparam InterfaceType The interface class to proxy. The proxy inherits from
 * this interface.
 */
template <typename InterfaceType>
class GOInterfaceProxy : public InterfaceType {
private:
  InterfaceType *p_target = nullptr;

public:
  virtual ~GOInterfaceProxy() = default;

  /**
   * Connects the proxy to a target object implementing the interface.
   * @param pTarget Pointer to the target object. Can be nullptr to disconnect.
   */
  inline void Connect(InterfaceType *pTarget) noexcept { p_target = pTarget; }

  /**
   * Disconnects the proxy from the target object.
   */
  inline void Disconnect() noexcept { p_target = nullptr; }

  /**
   * Checks if the proxy is connected to a target.
   * @return true if connected, false otherwise.
   */
  inline constexpr bool IsConnected() const noexcept {
    return p_target != nullptr;
  }

protected:
  // ========== SAFE VERSIONS (always safe) ==========

  /**
   * Safely forwards a void call to the target object. Does nothing if not
   * connected. Use for optional operations that can be skipped.
   *
   * @param func Pointer-to-member-function to call (must return void)
   * @param args Arguments to forward to the function
   *
   * Example:
   * \code
   * ForwardCall(&ISomeInterface::OptionalMethod, arg1, arg2);
   * \endcode
   */
  template <typename Func, typename... Args>
  void ForwardCall(Func &&func, Args &&...args) const {
    if (p_target)
      (p_target->*func)(std::forward<Args>(args)...);
  }

  /**
   * Non-const version of ForwardCall for void methods.
   */
  template <typename Func, typename... Args>
  void ForwardCall(Func &&func, Args &&...args) {
    if (p_target)
      (p_target->*func)(std::forward<Args>(args)...);
  }

  /**
   * Safely forwards a call to the target object with a default return value.
   * Returns the default value if not connected.
   * Use for optional operations that need to return a sensible default.
   *
   * @param func Pointer-to-member-function to call
   * @param defaultValue Value to return if proxy is not connected
   * @param args Arguments to forward to the function
   * @return The result of the function call, or defaultValue if not connected
   *
   * Example:
   * \code
   * // Returns 0 if not connected
   * return ForwardCall(&ISomeInterface::GetValue, 0, arg1, arg2);
   * \endcode
   */
  template <typename Func, typename DefaultType, typename... Args>
  auto ForwardCall(Func &&func, DefaultType &&defaultValue, Args &&...args)
    const -> decltype((p_target->*func)(std::forward<Args>(args)...)) {
    return p_target ? (p_target->*func)(std::forward<Args>(args)...)
                    : std::forward<DefaultType>(defaultValue);
  }

  /**
   * Non-const version of ForwardCall with default value.
   */
  template <typename Func, typename DefaultType, typename... Args>
  auto ForwardCall(Func &&func, DefaultType &&defaultValue, Args &&...args)
    -> decltype((p_target->*func)(std::forward<Args>(args)...)) {
    return p_target ? (p_target->*func)(std::forward<Args>(args)...)
                    : std::forward<DefaultType>(defaultValue);
  }
};

#endif /* GOINTERFACEPROXY_H */
