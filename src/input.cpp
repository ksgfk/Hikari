#include <hikari/input.h>

#include <hikari/opengl_header.h>

#include <unordered_map>

namespace Hikari {
static std::unordered_map<const void*, Input*> _scrollRowCallback;

Input::Input() noexcept = default;

void Input::Update(const void* handle) {
  GLFWwindow* window = (GLFWwindow*)handle;
  auto [_, isInsert] = _scrollRowCallback.try_emplace(handle, this);
  if (isInsert) {
    glfwSetScrollCallback(window, [](auto win, auto x, auto y) {
      auto input = _scrollRowCallback[win];
      input->_scrollDelta.X() += (float)x;
      input->_scrollDelta.Y() += (float)y;
    });
  }

  double x, y;
  glfwGetCursorPos(window, &x, &y);
  _mousePos = Vector2f((float)x, (float)y);

  _scrollNowDelta = _scrollDelta;
  _scrollDelta = Vector2f(0, 0);

  KeyStateArray nowKeyState{};
  for (size_t i = GLFW_KEY_A; i <= GLFW_KEY_Z; i++) {
    int state = glfwGetKey(window, (int)i);
    if (state == GLFW_PRESS) {
      nowKeyState[i - GLFW_KEY_A] = true;
    } else {
      nowKeyState[i - GLFW_KEY_A] = false;
    }
  }
  for (size_t i = 0; i < nowKeyState.size(); i++) {
    auto p = _keyNowState[i];
    auto n = nowKeyState[i];
    _keyDownState[i] = false;
    _keyUpState[i] = false;
    if (!p && n) {
      _keyDownState[i] = true;
    }
    if (p && !n) {
      _keyUpState[i] = true;
    }
    _keyNowState[i] = n;
  }

  ButtonStateArray nowBtnState{};
  for (size_t i = GLFW_MOUSE_BUTTON_LEFT; i <= GLFW_MOUSE_BUTTON_MIDDLE; i++) {
    int state = glfwGetMouseButton(window, (int)i);
    if (state == GLFW_PRESS) {
      nowBtnState[i - GLFW_MOUSE_BUTTON_LEFT] = true;
    } else {
      nowBtnState[i - GLFW_MOUSE_BUTTON_LEFT] = false;
    }
  }
  for (size_t i = 0; i < nowBtnState.size(); i++) {
    auto p = _msNowState[i];
    auto n = nowBtnState[i];
    _msDownState[i] = false;
    _msUpState[i] = false;
    if (!p && n) {
      _msDownState[i] = true;
    }
    if (p && !n) {
      _msUpState[i] = true;
    }
    _msNowState[i] = n;
  }
}

bool Input::GetKeyDown(KeyCode keyCode) const { return _keyDownState[(int)keyCode]; }

bool Input::GetKeyUp(KeyCode keyCode) const { return _keyUpState[(int)keyCode]; }

bool Input::GetKey(KeyCode keyCode) const { return _keyNowState[(int)keyCode]; }

bool Input::GetMouseDown(MouseButton msBtn) const { return _msDownState[(int)msBtn]; }

bool Input::GetMouseUp(MouseButton msBtn) const { return _msUpState[(int)msBtn]; }

bool Input::GetMouse(MouseButton msBtn) const { return _msNowState[(int)msBtn]; }

Vector2f Input::GetCursePos() const { return _mousePos; }

Vector2f Input::GetScrollDelta() const { return _scrollNowDelta; }
}  // namespace Hikari