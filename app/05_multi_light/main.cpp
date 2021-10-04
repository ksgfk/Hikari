#include <hikari/application.h>

using namespace Hikari;

const char vs[] = R"glsl(
#version 330 core

#ifndef HIKARI_TRANSFORM_INCLUDED
#define HIKARI_TRANSFORM_INCLUDED

uniform mat4 a_ObjectToWorld;

layout(std140) uniform HikariTransform {
  mat4 a_MatrixV;
  mat4 a_MatrixP;
  mat4 a_MatrixN;
  mat4 a_MatrixVP;
};

#endif

#ifndef HIKARI_LIGHT_INCLUDED
#define HIKARI_LIGHT_INCLUDED

#ifndef MAX_LIGHT_COUNT //默认最多8盏灯
#define MAX_LIGHT_COUNT 8
#endif

layout(std140) uniform HikariLight {
  vec3 a_LightRadiance[MAX_LIGHT_COUNT];
  vec3 a_LightDirection[MAX_LIGHT_COUNT];
};

#endif

void main() {}
)glsl";
const char fs[] = R"glsl(
#version 330 core
void main() {}
)glsl";

class Dummy : public RenderPass {
 public:
  Dummy() : RenderPass("Dummy", 0) {}

  void OnStart() override {
    SetProgram(vs, fs, {});
  }

  void OnUpdate() override {
    auto& prog = GetProgram();
    ActivePipelineConfig();
    ActiveProgram();
  }
};

int main() {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Multi Light"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreatePass<Dummy>();
  app.Awake();
  app.Run();
  return 0;
}