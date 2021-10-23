#include <iostream>

#include <hikari/application.h>

using namespace Hikari;

class Cube : public GameObject {
 public:
  Cube() : GameObject("cube") {}
  void OnStart() override {
    ImmutableModel cubeModel = ImmutableModel::CreateCube("cube", 1);
    CreateVbo(cubeModel, Vbo);
    VertexCount = int(cubeModel.GetIndexCount());
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  int VertexCount{};
};

class Sphere : public GameObject {
 public:
  Sphere() : GameObject("sphere") {}
  Sphere(int i, const Vector3f& pos) : GameObject(std::string("sphere") + std::to_string(i)) {
    GetTransform().Position = pos;
  }
  void OnStart() override {
    if (IndexCount == 0) {
      ImmutableModel sphereModel = ImmutableModel::CreateSphere("sphere", 0.3f, 32);
      CreateVboIbo(sphereModel, Vbo, Ibo);
      IndexCount = int(sphereModel.GetIndexCount());
    }
  }
  static std::shared_ptr<BufferOpenGL> Vbo;
  static std::shared_ptr<BufferOpenGL> Ibo;
  static int IndexCount;
};

std::shared_ptr<BufferOpenGL> Sphere::Vbo;
std::shared_ptr<BufferOpenGL> Sphere::Ibo;
int Sphere::IndexCount = 0;

class ClearPass : public RenderPass {
 public:
  ClearPass() : RenderPass("Clear Pass", 0) {}
  void OnUpdate() override {
    GetContext().ClearColorAndDepth();
  }
};

class MicrofacetPass : public RenderPass {
 public:
  constexpr static int Cnt = 5;
  constexpr static int Cnt2 = Cnt * Cnt;

  MicrofacetPass() : RenderPass("Cook Torrance Pass", 1) {}

  void OnStart() override {
    LoadProgram("cook_torrance.vert", "cook_torrance.frag", {POSITION(), NORMAL()});

    albedo = Vector3f(1.0f, 1.0f, 1.0f);
    for (size_t i = 1; i < Cnt; i++) {
      metallic[i] = (1 / (float(Cnt) - 1)) * i;
      roughness[i] = (1 / (float(Cnt) - 1)) * i;
    }
    metallic[0] = 0.1f;
    roughness[0] = 0.1f;
  }

  void OnPostStart() override {
    convSky = GetApp().GetSharedObject<std::shared_ptr<TextureOpenGL>>("convSkyBox");

    for (int i = 0; i < Cnt2; i++) {
      _spheres[i] = GetApp().GetGameObject<Sphere>(std::string("sphere") + std::to_string(i + 1));
    }
  }

  void OnUpdate() override {
    ActivePipelineConfig();
    ActiveProgram();
    SetViewportFullFrameBuffer();
    GetProgram()->UniformCubeMap("u_IrradianceMap", BindTexture(*convSky));
    GetProgram()->UniformVec3("u_metal.Albedo", albedo.GetAddress());
    for (size_t i = 0; i < Cnt; i++) {
      GetProgram()->UniformFloat("u_metal.Roughness", roughness[i]);
      for (size_t j = 0; j < Cnt; j++) {
        auto& sphere = _spheres[i * 5 + j];
        GetProgram()->UniformFloat("u_metal.Metallic", metallic[j]);
        SetModelMatrix(*sphere);
        SetVertexBuffer(sphere->Vbo, GetVertexPosPNT());
        SetVertexBuffer(sphere->Vbo, GetVertexNormalPNT());
        SetIndexBuffer(sphere->Ibo);
        DrawIndexed(sphere->IndexCount, 0);
      }
    }
  }

  std::shared_ptr<TextureOpenGL> convSky;
  Vector3f albedo{};
  std::array<std::shared_ptr<Sphere>, Cnt2> _spheres;
  std::array<float, 5> metallic{};
  std::array<float, 5> roughness{};
};

class SkyboxPass : public RenderPass {
 public:
  SkyboxPass() : RenderPass("Skybox", 2) {}

  void OnStart() override {
    _cube = GetApp().GetGameObject<Cube>("cube");

    ImmutableHdrTexture tex("hdr", GetApp().GetAssetPath() / "skybox_pillars_4k" / "pillars_4k.hdr", true);
    Texture2dDescriptorOpenGL hdrDesc;
    hdrDesc.Wrap = WrapMode::Clamp;
    hdrDesc.MinFilter = FilterMode::Bilinear;
    hdrDesc.MagFilter = FilterMode::Bilinear;
    hdrDesc.MipMapLevel = 0;
    hdrDesc.TextureFormat = PixelFormat::RGB32F;
    hdrDesc.Width = tex.GetWidth();
    hdrDesc.Height = tex.GetHeight();
    hdrDesc.DataFormat = tex.GetChannel() == 3 ? ImageDataFormat::RGB : ImageDataFormat::RGBA;
    hdrDesc.DataType = ImageDataType::Float32;
    hdrDesc.DataPtr = tex.GetData();
    TextureCubeMapDescriptorOpenGL dyMapDesc;
    dyMapDesc.Wrap = WrapMode::Clamp;
    dyMapDesc.MinFilter = FilterMode::Bilinear;
    dyMapDesc.MagFilter = FilterMode::Bilinear;
    dyMapDesc.MipMapLevel = 0;
    dyMapDesc.TextureFormat = PixelFormat::RGB32F;
    dyMapDesc.Width = 1024;
    dyMapDesc.Height = 1024;
    for (int i = 0; i < 6; i++) {
      dyMapDesc.DataFormat[i] = ImageDataFormat::RGB;
      dyMapDesc.DataType[i] = ImageDataType::Float32;
      dyMapDesc.DataPtr[i] = nullptr;
    }
    std::cout << "converting pillars_4k.hdr to cube map...";
    _skybox = GetContext().ConvertSphericalToCubemap(hdrDesc, dyMapDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    TextureCubeMapDescriptorOpenGL convDesc;
    convDesc.Wrap = WrapMode::Clamp;
    convDesc.MinFilter = FilterMode::Bilinear;
    convDesc.MagFilter = FilterMode::Bilinear;
    convDesc.MipMapLevel = 0;
    convDesc.TextureFormat = PixelFormat::RGB32F;
    convDesc.Width = 64;
    convDesc.Height = 64;
    for (int i = 0; i < 6; i++) {
      convDesc.DataFormat[i] = ImageDataFormat::RGB;
      convDesc.DataType[i] = ImageDataType::Float32;
      convDesc.DataPtr[i] = nullptr;
    }
    std::cout << "generating irradiance convolution for pillars_4k.hdr...";
    _conv = GetContext().GenIrradianceConvolutionCubemap(_skybox, convDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    LoadProgram("skybox.vert", "skybox.frag", {POSITION()});
    PipelineState state{};
    state.Depth.Comparison = DepthComparison::LessEqual;
    SetPipelineState(state);

    GetApp().SetSharedObject("convSkyBox", _conv);
  }

  void OnUpdate() override {
    auto& prog = GetProgram();
    auto& ctx = GetContext();
    SetViewportFullFrameBuffer();
    ActivePipelineConfig();
    ActiveProgram();
    prog->UniformMat4("view", GetCamera()->GetSkyboxViewMatrix().GetAddress());
    prog->UniformMat4("projection", GetCamera()->GetProjectionMatrix().GetAddress());
    prog->UniformCubeMap("u_skybox", BindTexture(*_skybox));
    SetVertexBuffer(*(_cube->Vbo), GetVertexPosPNT());
    Draw(_cube->VertexCount, 0);
  }

 private:
  std::shared_ptr<TextureOpenGL> _skybox;
  std::shared_ptr<TextureOpenGL> _conv;
  std::shared_ptr<Cube> _cube;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Diffuse Irradiance"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ClearPass>();
  app.CreatePass<SkyboxPass>();
  app.CreatePass<MicrofacetPass>();
  app.Instantiate<Cube>();
  int cnt = 1;
  for (float i = -2; i <= 2; i++) {
    for (float j = -2; j <= 2; j++) {
      app.Instantiate<Sphere>(cnt, Vector3f{i, j, 0});
      cnt++;
    }
  }
  app.CreateLight<Light>("direction light 1", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{-1, -1, -1});
  app.CreateLight<Light>("direction light 2", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{0, -1, 1});
  app.CreateLight<Light>("direction light 3", LightType::Directional, Vector3f{1}, 1.0f, Vector3f{1, 1, -1});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}