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

class ClearPass : public RenderPass {
 public:
  ClearPass() : RenderPass("Clear Pass", 0) {}
  void OnUpdate() override {
    GetContext().ClearColorAndDepth();
  }
};

class PbrPass : public RenderPass {
 public:
  PbrPass() : RenderPass("PBR Pass", 1) {
    albedo = {1.0f, 1.0f, 0.0f};
  }

  void OnStart() override {
    LoadProgram("cook_torrance.vert", "cook_torrance.frag", {POSITION(), NORMAL()});

    ImmutableHdrTexture env("hdr", GetApp().GetAssetPath() / "skybox_pillars_4k" / "pillars_4k.hdr", true);
    Texture2dDescriptorOpenGL hdrDesc;
    hdrDesc.Wrap = WrapMode::Clamp;
    hdrDesc.MinFilter = FilterMode::Bilinear;
    hdrDesc.MagFilter = FilterMode::Bilinear;
    hdrDesc.MipMapLevel = 0;
    hdrDesc.TextureFormat = PixelFormat::RGB32F;
    hdrDesc.Width = env.GetWidth();
    hdrDesc.Height = env.GetHeight();
    hdrDesc.DataFormat = env.GetChannel() == 3 ? ImageDataFormat::RGB : ImageDataFormat::RGBA;
    hdrDesc.DataType = ImageDataType::Float32;
    hdrDesc.DataPtr = env.GetData();
    TextureCubeMapDescriptorOpenGL skyDesc;
    skyDesc.Wrap = WrapMode::Clamp;
    skyDesc.MinFilter = FilterMode::Bilinear;
    skyDesc.MagFilter = FilterMode::Bilinear;
    skyDesc.MipMapLevel = 0;
    skyDesc.TextureFormat = PixelFormat::RGB32F;
    skyDesc.Width = 1024;
    skyDesc.Height = 1024;
    for (int i = 0; i < 6; i++) {
      skyDesc.DataFormat[i] = ImageDataFormat::RGB;
      skyDesc.DataType[i] = ImageDataType::Float32;
      skyDesc.DataPtr[i] = nullptr;
    }
    std::cout << "converting pillars_4k.hdr to cube map...";
    skybox = GetContext().ConvertSphericalToCubemap(hdrDesc, skyDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    TextureCubeMapDescriptorOpenGL skyConvDesc;
    skyConvDesc.Wrap = WrapMode::Clamp;
    skyConvDesc.MinFilter = FilterMode::Bilinear;
    skyConvDesc.MagFilter = FilterMode::Bilinear;
    skyConvDesc.MipMapLevel = 0;
    skyConvDesc.TextureFormat = PixelFormat::RGB32F;
    skyConvDesc.Width = 64;
    skyConvDesc.Height = 64;
    for (int i = 0; i < 6; i++) {
      skyConvDesc.DataFormat[i] = ImageDataFormat::RGB;
      skyConvDesc.DataType[i] = ImageDataType::Float32;
      skyConvDesc.DataPtr[i] = nullptr;
    }
    std::cout << "generating irradiance convolution for pillars_4k.hdr...";
    skyConv = GetContext().GenIrradianceConvolutionCubemap(skybox, skyConvDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    TextureCubeMapDescriptorOpenGL filterDesc;
    filterDesc.Wrap = WrapMode::Clamp;
    filterDesc.TextureFormat = PixelFormat::RGB32F;
    filterDesc.Width = 1024;
    filterDesc.Height = 1024;
    for (int i = 0; i < 6; i++) {
      filterDesc.DataFormat[i] = ImageDataFormat::RGB;
      filterDesc.DataType[i] = ImageDataType::Float32;
      filterDesc.DataPtr[i] = nullptr;
    }
    std::cout << "prefilter pillars_4k.hdr...";
    skyFilter = GetContext().PrefilterEnvMap(skybox, filterDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;
    maxLod = TextureOpenGL::CalcMipmapLevels(0, 512, true);

    Texture2dDescriptorOpenGL lutDesc;
    lutDesc.Wrap = WrapMode::Clamp;
    lutDesc.MinFilter = FilterMode::Bilinear;
    lutDesc.MagFilter = FilterMode::Bilinear;
    lutDesc.MipMapLevel = 0;
    lutDesc.TextureFormat = PixelFormat::RG32F;
    lutDesc.Width = 1024;
    lutDesc.Height = 1024;
    lutDesc.DataFormat = ImageDataFormat::RG;
    lutDesc.DataType = ImageDataType::Float32;
    std::cout << "precompute brdf lut...";
    brdfLut = GetContext().PrecomputeBrdfLut(lutDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    GetApp().SetSharedObject("skybox", skyConv);
  }

  void OnPostStart() override {
    for (int i = 0; i < 25; i++) {
      spheres[i] = GetApp().GetGameObject<Sphere>(std::string("sphere") + std::to_string(i));
    }
  }

  void OnUpdate() override {
    SetViewportFullFrameBuffer();
    ActivePipelineConfig();
    ActiveProgram();
    GetProgram()->UniformCubeMap("u_IrradianceMap", BindTexture(*skyConv));
    GetProgram()->UniformCubeMap("u_PrefilterMap", BindTexture(*skyFilter));
    GetProgram()->UniformTexture2D("u_BrdfLut", BindTexture(*brdfLut));
    GetProgram()->UniformInt("u_MaxLod", maxLod);
    GetProgram()->UniformVec3("u_metal.Albedo", albedo.GetAddress());
    for (size_t i = 0; i < 5; i++) {
      GetProgram()->UniformFloat("u_metal.Roughness", i == 0 ? 0.05f : 0.25f * i);
      for (size_t j = 0; j < 5; j++) {
        auto& sphere = spheres[i * 5 + j];
        GetProgram()->UniformFloat("u_metal.Metallic", j == 0 ? 0.05f : 0.25f * j);
        SetModelMatrix(*sphere);
        SetVertexBuffer(sphere->sphere->GetVbo(), GetVertexPosPNT());
        SetVertexBuffer(sphere->sphere->GetVbo(), GetVertexNormalPNT());
        SetIndexBuffer(sphere->sphere->GetIbo());
        sphere->sphere->Draw(*this);
      }
    }
  }

  std::shared_ptr<TextureOpenGL> skybox;
  std::shared_ptr<TextureOpenGL> skyConv;
  std::shared_ptr<TextureOpenGL> skyFilter;
  std::shared_ptr<TextureOpenGL> brdfLut;
  std::shared_ptr<Sphere> spheres[25];
  Vector3f albedo{};
  int maxLod{};
};

class SkyboxPass : public RenderPass {
 public:
  SkyboxPass() : RenderPass("Skybox Pass", 2) {}

  void OnStart() override {
    LoadProgram("skybox.vert", "skybox.frag", {POSITION()});

    PipelineState state{};
    state.Depth.Comparison = DepthComparison::LessEqual;
    SetPipelineState(state);
  }

  void OnPostStart() override {
    skybox = GetApp().GetSharedObject<std::shared_ptr<TextureOpenGL>>("skybox");
    cube = GetApp().GetRenderable("cube");
  }

  void OnUpdate() override {
    SetViewportFullFrameBuffer();
    ActivePipelineConfig();
    ActiveProgram();
    GetProgram()->UniformCubeMap("u_skybox", BindTexture(*skybox));
    SetVertexBuffer(cube->GetVbo(), GetVertexPosPNT());
    cube->Draw(*this);
  }

  std::shared_ptr<Renderable> cube;
  std::shared_ptr<TextureOpenGL> skybox;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari PBR Environment"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ClearPass>();
  app.CreatePass<PbrPass>();
  app.CreatePass<SkyboxPass>();
  app.CreateRenderable<RenderableCube>("cube", 1.0f);
  app.CreateRenderable<RenderableSphere>("sphere", 0.35f, 32);
  app.CreateCamera<PerspectiveCamera>();
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      app.Instantiate<Sphere>(i * 5 + j, Vector3f{i - 2, j - 2, 0});
    }
  }
  app.CreateLight<Light>("direction light 1", LightType::Directional, Vector3f{1}, 10.0f, Vector3f{0, -1, 1});
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}