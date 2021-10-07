#include <random>

#include <hikari/application.h>

using namespace Hikari;

class Cube : public GameObject {
 public:
  Cube() : GameObject("cube") {}
  void OnStart() override {
    ImmutableModel cubeModel = ImmutableModel::CreateSphere("sphere", 1, 32);
    CreateVbo(cubeModel, Vbo);
    VertexCount = int(cubeModel.GetIndexCount());
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  int VertexCount{};
};

class BlinnPass : public RenderPass {
 public:
  BlinnPass() : RenderPass("blinn pass", 0) {}

  void OnStart() override {
    LoadProgram("blinn.vert", "blinn.frag", {POSITION0(), NORMAL0()});
    _cube = GetApp().GetGameObject<Cube>("cube");

    _kd = Vector3f(0.2f, 0.2f, 0.2f);
    _ks = Vector3f{1.0f};
    _shiness = 64;
  }

  void OnUpdate() override {
    auto& ctx = GetContext();
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    GetProgram()->UniformVec3("u_blinn.kd", _kd.GetAddress());
    GetProgram()->UniformVec3("u_blinn.ks", _ks.GetAddress());
    GetProgram()->UniformFloat("u_blinn.shiness", _shiness);
    SetModelMatrix(*_cube);
    SetVertexBuffer(_cube->Vbo, GetVertexLayoutPositionPNT());
    SetVertexBuffer(_cube->Vbo, GetVertexLayoutNormalPNT());
    Draw(_cube->VertexCount, 0);
  }

 private:
  std::shared_ptr<Cube> _cube;
  Vector3f _kd{};
  Vector3f _ks{};
  float _shiness{};
};

static std::mt19937 eng(uint32_t(std::chrono::system_clock::now().time_since_epoch().count()));
static std::uniform_real_distribution<float> dist;

class RandomOrbitLight : public Light {
 public:
  RandomOrbitLight(const std::string& name) : Light(name) {
    Type = LightType::Directional;
    Color = {dist(eng), dist(eng), dist(eng)};
    Intensity = dist(eng) * (1 + dist(eng));
    _speed = 0.0025f;
    auto b = dist(eng) >= 0.5f ? -1 : 1;
    auto c = dist(eng) >= 0.5f ? -1 : 1;
    p = dist(eng) * b;
    t = dist(eng) * c;
    auto d = dist(eng) >= 0.5f ? -1 : 1;
    auto e = dist(eng) >= 0.5f ? -1 : 1;
    _dir = {dist(eng) * d, dist(eng) * e};
  }

  void OnUpdate() override {  //瞎写的
    float factor = 2 * PI * _speed;
    t += _dir.X() * factor;
    p += _dir.Y() * factor;
    Vector3f offset(0);
    offset.X() = std::sin(p) * std::sin(t);
    offset.Y() = std::cos(p);
    offset.Z() = std::sin(p) * std::cos(t);
    Direction = Normalize(offset);
  }

 private:
  float _speed{};
  float p{};
  float t{};
  Vector2f _dir{};
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Multi Light"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<BlinnPass>();
  app.Instantiate<Cube>();
  for (int i = 0; i < 8; i++) {
    app.CreateLight<RandomOrbitLight>("direction light " + std::to_string(i));
  }
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}