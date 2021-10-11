#include <iostream>
#include <hikari/application.h>

using namespace Hikari;

const float vertices[] = {
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f};
const float color[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f};
unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3};
const char vs[] = R"glsl(
#version 330 core
in vec3 aPos;
in vec3 aColor;
out vec3 vColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main() {
  gl_Position = proj * view * model * vec4(aPos, 1.0f);
  vColor = aColor;
}
)glsl";
const char fs[] = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 vColor;
void main(){
  FragColor = vec4(vColor, 1.0f);
}
)glsl";

class Triangle : public GameObject {
 public:
  Triangle() : GameObject("triangle") {}
  ~Triangle() override = default;
  void OnStart() override {
    auto size = sizeof(vertices) / sizeof(float) / 3;
    std::vector<float> verties;
    for (size_t i = 0; i < size; i++) {
      for (size_t j = 0; j < 3; j++) {
        verties.push_back(vertices[i * 3 + j]);
      }
      for (size_t j = 0; j < 3; j++) {
        verties.push_back(color[i * 3 + j]);
      }
    }
    Vbo = GetApp().GetContext().CreateVertexBuffer(verties.data(), verties.size() * sizeof(float));
    Ibo = GetApp().GetContext().CreateIndexBuffer(indices, sizeof(indices));
  }

  std::shared_ptr<BufferOpenGL> Vbo;
  std::shared_ptr<BufferOpenGL> Ibo;
};

class ColorPass : public RenderPass {
 public:
  ColorPass() : RenderPass("color", 0) {}
  ~ColorPass() override = default;
  void OnStart() override {
    ShaderAttributeLayouts layouts = {{"aPos", SemanticType::Vertex, 0}, {"aColor", SemanticType::Color, 0}};
    SetProgram(vs, fs, layouts);
    _tri = GetApp().GetGameObject<Triangle>("triangle");
  }
  void OnUpdate() override {
    auto& prog = GetProgram();
    auto& ctx = GetContext();
    ctx.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    ctx.ClearColor();
    ctx.SetViewport(0, 0, GetFrameBufferWidth(), GetFrameBufferHeight());
    ActiveProgram();
    prog->UniformMat4("model", _tri->GetTransform().ObjectToWorldMatrix().GetAddress());
    prog->UniformMat4("view", GetCamera()->GetViewMatrix().GetAddress());
    prog->UniformMat4("proj", GetCamera()->GetProjectionMatrix().GetAddress());
    SetVertexBuffer(*(_tri->Vbo), {{SemanticType::Vertex, 0}, 3 * sizeof(float) * 2, 0});
    SetVertexBuffer(*(_tri->Vbo), {{SemanticType::Color, 0}, 3 * sizeof(float) * 2, 3 * sizeof(float)});
    SetIndexBuffer(*(_tri->Ibo));
    DrawIndexed(sizeof(indices) / sizeof(unsigned int), 0);
  }

 private:
  std::shared_ptr<Triangle> _tri;
};

int main() {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Simple Triangle"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreateCamera<OrthoCamera>();
  app.Instantiate<Triangle>();
  app.CreatePass<ColorPass>();
  app.Awake();
  app.Run();
}