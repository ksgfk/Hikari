#include <iostream>

#include <hikari/application.h>

using namespace Hikari;

class Sphere : public GameObject {
 public:
  Sphere(int i, const Vector3f& pos) : GameObject(std::string("sphere") + std::to_string(i)) {
    GetTransform().Position = pos;
  }

  void OnStart() override {
    sphere = GetApp().GetRenderable("sphere");
  }

  std::shared_ptr<Renderable> sphere;
};

class ColorPass : public RenderPass {
 public:
  ColorPass() : RenderPass("Color Pass", 0) {}

  void OnStart() override {
    LoadProgram("pbr.vert", "pbr.frag", {POSITION(), TANGENT(), NORMAL(), TEXCOORD0()});
    auto& ctx = GetContext();
    auto file = GetApp().GetAssetPath() / "copper-rock1-bl";
    albedo = ctx.LoadBitmap2D(file / "copper-rock1-alb.png", WrapMode::Clamp, FilterMode::Bilinear, PixelFormat::RGB8);
    normal = ctx.LoadBitmap2D(file / "copper-rock1-normal.png", WrapMode::Clamp, FilterMode::Bilinear, PixelFormat::RGB8);
    rough = ctx.LoadBitmap2D(file / "copper-rock1-rough.png", WrapMode::Clamp, FilterMode::Bilinear, PixelFormat::RGB8);
    metallic = 0.5f;
  }

  void OnPostStart() override {
    sphere1 = GetApp().GetGameObject<Sphere>("sphere1");
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    auto& prog = GetProgram();
    ctx.ClearColorAndDepth();
    SetViewportFullFrameBuffer();
    ActivePipelineConfig();
    ActiveProgram();
    prog->UniformTexture2D("u_Albedo", BindTexture(*albedo));
    prog->UniformTexture2D("u_Roughness", BindTexture(*rough));
    prog->UniformTexture2D("u_NormalMap", BindTexture(*normal));
    prog->UniformFloat("u_Metallic", metallic);
    SetModelMatrix(*sphere1);
    SetVertexBuffer(sphere1->sphere->GetVbo(), GetVertexPosPTNT());
    SetVertexBuffer(sphere1->sphere->GetVbo(), GetVertexTanPTNT());
    SetVertexBuffer(sphere1->sphere->GetVbo(), GetVertexNormalPTNT());
    SetVertexBuffer(sphere1->sphere->GetVbo(), GetVertexTexPTNT(0));
    SetIndexBuffer(sphere1->sphere->GetIbo());
    sphere1->sphere->Draw(*this);
  }

  std::shared_ptr<Sphere> sphere1;
  std::shared_ptr<TextureOpenGL> albedo;
  std::shared_ptr<TextureOpenGL> normal;
  std::shared_ptr<TextureOpenGL> rough;
  float metallic{};
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Normal Map"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ColorPass>();
  app.CreateRenderable<RenderableSphere>("sphere", 1.0f, 32, true);
  app.Instantiate<Sphere>(1, Vector3f{0, 0, 0});
  app.CreateLight<Light>("direction light 1", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{0, -1, 1});
  app.CreateLight<Light>("direction light 2", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{0, 1, -1});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}