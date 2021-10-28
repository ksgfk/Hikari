#include <iostream>
#include <random>

#include <hikari/application.h>

using namespace Hikari;

static std::mt19937 gen;
static std::normal_distribution<float> d;

class MoveLight : public Light {
 public:
  MoveLight(int i, Vector3f c, float inten, Vector3f p)
      : Light(std::string("p") + std::to_string(i), LightType::Point, c, inten, p) {
    dir = Normalize(Vector3f{d(gen), d(gen), d(gen)});
    speed = d(gen) * 0.5f;
  }

  void OnUpdate() override {
    auto t = GetApp().GetDeltaTime();
    auto delta = dir * Vector3f(speed) * Vector3f(t);
    if (Length(delta) > 10) {
      delta = {};
    }
    Direction += delta;
    if (std::abs(Direction.X()) > 50) {
      Direction.X() = std::signbit(Direction.X()) * 50.0f;
      dir.X() *= -1;
    }
    if (std::abs(Direction.Y()) > 50) {
      Direction.Y() = std::signbit(Direction.Y()) * 50.0f;
      dir.Y() *= -1;
    }
    if (std::abs(Direction.Z()) > 50) {
      Direction.Z() = std::signbit(Direction.Z()) * 50.0f;
      dir.Z() *= -1;
    }
  }

  Vector3f dir;
  float speed;
};

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

class Quad : public GameObject {
 public:
  Quad() : GameObject("quad") {
    GetTransform().Position = {0.5f, 0.5f, 0};
  }

  void OnStart() override {
    quad = GetApp().GetRenderable("quad");
  }

  std::shared_ptr<Renderable> quad;
};

class GPass : public RenderPass {
 public:
  GPass() : RenderPass("G Pass", 0) {
    albedo = Vector3f{1, 1, 1};
  }

  void OnStart() override {
    LoadProgram("gbuffer.vert", "gbuffer.frag", {POSITION(), NORMAL()});
    std::vector<GBufferLayout> layout;
    layout.emplace_back(GBufferLayout{"g_Pos", PixelFormat::RGB32F, ImageDataFormat::RGB, ImageDataType::Float32});
    layout.emplace_back(GBufferLayout{"g_Normal", PixelFormat::RGB32F, ImageDataFormat::RGB, ImageDataType::Float32});
    layout.emplace_back(GBufferLayout{"g_Albedo", PixelFormat::RGB32F, ImageDataFormat::RGB, ImageDataType::Float32});
    layout.emplace_back(GBufferLayout{"g_Param", PixelFormat::RGB32F, ImageDataFormat::RGB, ImageDataType::Float32});
    gbuffer = GetContext().CreateGBuffer({1920, 1080}, layout, true);
  }

  void OnPostStart() override {
    for (int i = 0; i < 441; i++) {
      target.emplace_back(GetApp().GetGameObject<Sphere>(std::string("sphere") + std::to_string(i)));
    }
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    ctx.SetViewport(0, 0, 1920, 1080);
    gbuffer->Frame->Bind();
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    auto& prog = GetProgram();
    GetProgram()->UniformVec3("u_pbr.Albedo", albedo.GetAddress());
    float all = float(target.size());
    for (size_t i = 0; i < target.size(); i++) {
      auto& o = target[i];
      auto v = (float(i) + 1.0f) / all;
      GetProgram()->UniformFloat("u_pbr.Roughness", v < 0.1f ? 0.1f : v);
      GetProgram()->UniformFloat("u_pbr.Metallic", (1 - v) < 0.1f ? 0.1f : (1 - v));
      SetModelMatrix(*o);
      SetVertexBuffer(o->sphere->GetVbo(), GetVertexPosPNT());
      SetVertexBuffer(o->sphere->GetVbo(), GetVertexNormalPNT());
      SetIndexBuffer(o->sphere->GetIbo());
      DrawIndexed(o->sphere->GetDrawCount(), 0);
    }
    gbuffer->Frame->Unbind();
  }

  std::vector<std::shared_ptr<Sphere>> target;
  std::unique_ptr<GBuffer> gbuffer;
  Vector3f albedo;
};

class ShadePass : public RenderPass {
 public:
  ShadePass() : RenderPass("Shade Pass", 1) {}

  void OnStart() override {
    LoadProgram("light.vert", "light.frag", {POSITION(), TEXCOORD0()});
  }

  void OnPostStart() override {
    g = GetApp().GetRenderPass<GPass>("G Pass");
    q = GetApp().GetGameObject<Quad>("quad");
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    SetViewportFullFrameBuffer();
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    auto& prog = GetProgram();
    auto& gbuffer = g->gbuffer;
    for (size_t i = 0; i < gbuffer->Buffers.size(); i++) {
      prog->UniformTexture2D(gbuffer->Layouts[i].Stage, BindTexture(*gbuffer->Buffers[i]));
    }
    SetVertexBuffer(q->quad->GetVbo(), GetVertexPosPNT());
    SetVertexBuffer(q->quad->GetVbo(), GetVertexTexPNT(0));
    Draw(q->quad->GetDrawCount(), 0);
  }

  std::shared_ptr<GPass> g;
  std::shared_ptr<Quad> q;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  gen = std::mt19937(uint32_t(app.GetRealTime()));
  app.SetWindowCreateInfo({"Hikari Deferred Shading"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<GPass>();
  app.CreatePass<ShadePass>();
  app.CreateRenderable<RenderableSphere>("sphere", 0.5f, 32);
  app.CreateRenderable<RenderableQuad>("quad", 1.0f);
  for (int i = 0; i <= 1024; i++) {
    app.Instantiate<Sphere>(i, Vector3f{-10 + d(gen) * 20, -10 + d(gen) * 20, -10 + d(gen) * 20});
  }
  app.Instantiate<Quad>();
  for (int i = 0; i <= 1024; i++) {
    app.CreateLight<MoveLight>(i,
                               Vector3f{d(gen), d(gen), d(gen)},
                               d(gen) * 5.0f,
                               Vector3f{-20 + d(gen) * 40, -20 + d(gen) * 40, -20 + d(gen) * 40});
  }
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.GetCamera().Camera->SetPosition({0, 0, -12});
  app.EnableImgui();
  app.Awake();
  app.Run();
  return 0;
}