#include <hikari/window.h>

#include <hikari/opengl_header.h>

#include <iostream>
#include <unordered_map>

namespace Hikari {

WindowCreateInfo::WindowCreateInfo() noexcept = default;

WindowCreateInfo::WindowCreateInfo(int w, int h, const std::string& title) noexcept
    : Width(w), Height(h), Title(title) {}

WindowCreateInfo::WindowCreateInfo(const std::string& title) noexcept
    : WindowCreateInfo(1280, 720, title) {}

NativeWindow::NativeWindow() noexcept = default;

NativeWindow::NativeWindow(NativeWindow&& other) noexcept {
  _handle = other._handle;
  other._handle = nullptr;
  //_resizeEvent = std::move(other._resizeEvent);
}

NativeWindow::~NativeWindow() noexcept {
  Destroy();
}

bool NativeWindow::IsValid() const {
  return _handle != nullptr;
}

void NativeWindow::Destroy() {
  if (_handle == nullptr) {
    return;
  }
  glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(_handle));
  _handle = nullptr;
}

void NativeWindow::PollEvents() const {
  glfwPollEvents();
}

void NativeWindow::SwapBuffers() const {
  glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(_handle));
}

bool NativeWindow::ShouldClose() const {
  return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(_handle));
}

const void* NativeWindow::GetHandle() const {
  return _handle;
}

void NativeWindow::GetWindowSize(int& width, int& height) const {
  auto glfwWindow = reinterpret_cast<GLFWwindow*>(_handle);
  glfwGetWindowSize(glfwWindow, &width, &height);
}

void NativeWindow::GetFrameBufferSize(int& width, int& height) const {
  auto glfwWindow = reinterpret_cast<GLFWwindow*>(_handle);
  glfwGetFramebufferSize(glfwWindow, &width, &height);
}

void NativeWindow::AddFramebufferResizeEvent(FramebufferResizeCallback&& callback) {
  _resizeEvent.emplace_back(std::move(callback));
}

void NativeWindow::RemoveFramebufferResizeEvent(const FramebufferResizeCallback& callback) {
  for (auto iter = _resizeEvent.begin(); iter != _resizeEvent.end(); iter++) {
    const auto& e = *iter;
    if (e.target<FramebufferResizeCallback>() == callback.target<FramebufferResizeCallback>()) {
      _resizeEvent.erase(iter);
      break;
    }
  }
}

}  // namespace Hikari