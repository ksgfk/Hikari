#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Hikari {
struct WindowCreateInfo {
  int Width = 1280;
  int Height = 720;
  std::string Title;
  WindowCreateInfo() noexcept;
  WindowCreateInfo(int w, int h, const std::string& title) noexcept;
  WindowCreateInfo(const std::string& title) noexcept;
};

/**
  * @brief 窗口
  */
class NativeWindow {
 public:
  using FramebufferResizeCallback = std::function<void(int, int)>;

  NativeWindow() noexcept;
  NativeWindow(const NativeWindow&) = delete;
  NativeWindow(NativeWindow&&) noexcept;
  ~NativeWindow() noexcept;
  bool IsValid() const;
  void Destroy();

  void GetWindowSize(int& width, int& height) const;
  void GetFrameBufferSize(int& width, int& height) const;

  void PollEvents() const;
  void SwapBuffers() const;
  bool ShouldClose() const;
  const void* GetHandle() const;
  void AddFramebufferResizeEvent(FramebufferResizeCallback&& callback);
  void RemoveFramebufferResizeEvent(const FramebufferResizeCallback& callback);

 private:
  void* _handle = nullptr;
  std::vector<FramebufferResizeCallback> _resizeEvent;
  friend class Application;
};

}  // namespace Hikari