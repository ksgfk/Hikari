#include <iostream>

#include <hikari/application.h>

using namespace Hikari;

class Sphere : public GameObject {
 public:
  Sphere() : GameObject("sphere") {}
  Sphere(int i, const Vector3f& pos) : GameObject(std::string("sphere") + std::to_string(i)) {
    GetTransform().Position = pos;
  }
  void OnStart() override {
    if (VertexCount == 0) {
      ImmutableModel cubeModel = ImmutableModel::CreateSphere("sphere", 0.25f, 32);
      CreateVboIbo(cubeModel, Vbo, Ibo);
      VertexCount = int(cubeModel.GetIndexCount());
    }
  }

  static std::shared_ptr<BufferOpenGL> Vbo;
  static std::shared_ptr<BufferOpenGL> Ibo;
  static int VertexCount;
};

std::shared_ptr<BufferOpenGL> Sphere::Vbo;
std::shared_ptr<BufferOpenGL> Sphere::Ibo;
int Sphere::VertexCount = 0;

class ClearPass : public RenderPass {
 public:
  ClearPass() : RenderPass("clear pass", 0) {}
  void OnUpdate() override {
    auto& ctx = GetContext();
    SetViewportFullFrameBuffer();
    ctx.ClearColorAndDepth();
  }
};

class BlinnPass : public RenderPass {
 public:
  BlinnPass() : RenderPass("blinn pass", 1) {}

  void OnStart() override {
    LoadProgram("blinn.vert", "blinn.frag", {POSITION0(), NORMAL0()});

    _sphere = GetApp().GetGameObject<Sphere>("sphere0");

    _kd = Vector3f(0.651f, 0.490f, 0.239f) * Vector3f{0.25f};
    _ks = Vector3f{1.0f};
    _shiness = 96;
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    ActivePipelineConfig();
    ActiveProgram();
    GetProgram()->UniformVec3("u_blinn.kd", _kd.GetAddress());
    GetProgram()->UniformVec3("u_blinn.ks", _ks.GetAddress());
    GetProgram()->UniformFloat("u_blinn.shiness", _shiness);
    SetModelMatrix(*_sphere);
    SetVertexBuffer(_sphere->Vbo, GetVertexLayoutPositionPNT());
    SetVertexBuffer(_sphere->Vbo, GetVertexLayoutNormalPNT());
    SetIndexBuffer(_sphere->Ibo);
    DrawIndexed(_sphere->VertexCount, 0);
  }

 private:
  std::shared_ptr<Sphere> _sphere;
  Vector3f _kd{};
  Vector3f _ks{};
  float _shiness{};
};

class MicrofacetCookTorrance : public RenderPass {
 public:
  constexpr static int Count = 25;

  MicrofacetCookTorrance() : RenderPass("microfacet pass", 1) {
    _albedo = Vector3f(0.651f, 0.490f, 0.239f);
  }
  MicrofacetCookTorrance(Vector3f albedo) : _albedo(albedo) {}

  void OnStart() override {
    LoadProgram("cook_torrance.vert", "cook_torrance.frag", {POSITION0(), NORMAL0()});

    for (int i = 0; i < Count; i++) {
      _spheres[i] = GetApp().GetGameObject<Sphere>(std::string("sphere") + std::to_string(i + 1));
    }

    for (size_t i = 1; i < 5; i++) {
      _metallic[i] = 0.25f * i;
      _roughness[i] = 0.25f * i;
    }
    _metallic[0] = 0.1f;
    _roughness[0] = 0.1f;
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    ActivePipelineConfig();
    ActiveProgram();
    GetProgram()->UniformVec3("u_metal.Albedo", _albedo.GetAddress());
    for (size_t i = 0; i < 5; i++) {
      GetProgram()->UniformFloat("u_metal.Roughness", _roughness[i]);
      for (size_t j = 0; j < 5; j++) {
        auto& sphere = _spheres[i * 5 + j];
        GetProgram()->UniformFloat("u_metal.Metallic", _metallic[j]);
        SetModelMatrix(*sphere);
        SetVertexBuffer(sphere->Vbo, GetVertexLayoutPositionPNT());
        SetVertexBuffer(sphere->Vbo, GetVertexLayoutNormalPNT());
        SetIndexBuffer(sphere->Ibo);
        DrawIndexed(sphere->VertexCount, 0);
      }
    }
  }

 private:
  std::array<std::shared_ptr<Sphere>, Count> _spheres;
  std::array<float, 5> _metallic{};
  std::array<float, 5> _roughness{};
  Vector3f _albedo{};
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Physical Based Rendering - Direct Light"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<BlinnPass>();
  app.CreatePass<MicrofacetCookTorrance>();
  app.CreatePass<ClearPass>();
  int cnt = 1;
  for (float i = -2; i <= 2; i++) {
    for (float j = -2; j <= 2; j++) {
      app.Instantiate<Sphere>(cnt, Vector3f{i, j, 0});
      cnt++;
    }
  }
  app.Instantiate<Sphere>(0, Vector3f{0, 0, -2.0f});
  app.CreateLight<Light>("direction light 1", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{0, -1, 1});
  app.CreateLight<Light>("direction light 2", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{1, -1.5, 1});
  app.CreateLight<Light>("direction light 3", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{-1, 1, 1});
  app.CreateLight<Light>("direction light 4", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{1, -1, 1});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.GetCamera().Camera->SetPosition({0, 0, -5});
  app.Awake();
  app.Run();
  return 0;
}