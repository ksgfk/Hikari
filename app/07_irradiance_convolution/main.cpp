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
  void OnStart() override {
    ImmutableModel sphereModel = ImmutableModel::CreateSphere("sphere", 1, 32);
    CreateVboIbo(sphereModel, Vbo, Ibo);
    IndexCount = int(sphereModel.GetIndexCount());
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  std::shared_ptr<BufferOpenGL> Ibo;
  int IndexCount{};
};

class ClearPass : public RenderPass {
 public:
  ClearPass() : RenderPass("Clear Pass", 0) {}
  void OnUpdate() override {
    GetContext().ClearColorAndDepth();
  }
};

class ColorPass : public RenderPass {
 public:
  ColorPass() : RenderPass("Color Pass", 1) {}

  void OnStart() override {
    LoadProgram("texture.vert", "texture.frag", {POSITION()});
  }

  void OnPostStart() override {
    convSky = GetApp().GetSharedObject<std::shared_ptr<TextureOpenGL>>("convSkyBox");
    _sphere = GetApp().GetGameObject<Sphere>("sphere");
  }

  void OnUpdate() override {
    ActivePipelineConfig();
    ActiveProgram();
    SetViewportFullFrameBuffer();
    SetModelMatrix(*_sphere);
    GetProgram()->UniformCubeMap("u_cube", BindTexture(*convSky));
    SetVertexBuffer(_sphere->Vbo, GetVertexPosPNT());
    SetIndexBuffer(_sphere->Ibo);
    DrawIndexed(_sphere->IndexCount, 0);
  }

  std::shared_ptr<Sphere> _sphere;
  std::shared_ptr<TextureOpenGL> convSky;
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
  app.SetWindowCreateInfo({"Hikari Irradiance Convolution"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ClearPass>();
  app.CreatePass<SkyboxPass>();
  app.CreatePass<ColorPass>();
  app.Instantiate<Cube>();
  app.Instantiate<Sphere>();
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}