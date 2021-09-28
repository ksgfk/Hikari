#include <iostream>
#include <hikari/application.h>

using namespace Hikari;

class EmptyPass : public RenderPass {
 public:
  EmptyPass() : RenderPass("empty", 0) {}
  void OnUpdate() override {
    HIKARI_CHECK_GL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
  }
};

int main() {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Hello World"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreatePass<EmptyPass>();
  app.Awake();
  app.Run();
  return 0;
}