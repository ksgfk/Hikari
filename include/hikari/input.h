#pragma once

#include <hikari/mathematics.h>

#include <array>

namespace Hikari {
enum class KeyCode {
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  SIZE
};

enum class MouseButton {
  Left,
  Right,
  Middle,
  SIZE
};

class Input {
 public:
  constexpr static size_t KeyCodeCount = (size_t)KeyCode::SIZE;
  constexpr static size_t MouseButtonCount = (size_t)MouseButton::SIZE;
  using KeyStateArray = std::array<bool, KeyCodeCount>;
  using ButtonStateArray = std::array<bool, MouseButtonCount>;

 public:
  Input() noexcept;
  void Update(const void* handle);
  bool GetKey(KeyCode keyCode) const;
  bool GetKeyDown(KeyCode keyCode) const;
  bool GetKeyUp(KeyCode keyCode) const;
  bool GetMouse(MouseButton msBtn) const;
  bool GetMouseDown(MouseButton msBtn) const;
  bool GetMouseUp(MouseButton msBtn) const;
  Vector2f GetCursePos() const;
  Vector2f GetScrollDelta() const;

 private:
  KeyStateArray _keyNowState{};
  KeyStateArray _keyDownState{};
  KeyStateArray _keyUpState{};
  ButtonStateArray _msNowState{};
  ButtonStateArray _msDownState{};
  ButtonStateArray _msUpState{};
  Vector2f _mousePos{};
  Vector2f _scrollNowDelta{};
  Vector2f _scrollDelta{};
};
}  // namespace Hikari