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
      ImmutableModel sphereModel = ImmutableModel::CreateSphere("sphere", 0.4f, 32);
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

class ColorPass : public RenderPass {
 public:
  ColorPass() : RenderPass("Color Pass", 1) {}

  void OnStart() override {
    LoadProgram("texture.vert", "texture.frag", {POSITION0(), NORMAL0()});
  }

  void OnPostStart() override {
    convSky = GetApp().GetSharedObject<std::shared_ptr<TextureOpenGL>>("convSkyBox");
    for (int i = 0; i < 5; i++) {
      sphere[i] = GetApp().GetGameObject<Sphere>(std::string("sphere") + std::to_string(i));
    }
  }

  void OnUpdate() override {
    ActivePipelineConfig();
    ActiveProgram();
    SetViewportFullFrameBuffer();
    for (int i = 0; i < 5; i++) {
      SetModelMatrix(*sphere[i]);
      GetProgram()->UniformCubeMap("u_cube", BindTexture(*convSky));
      GetProgram()->UniformInt("u_roughness", i);
      SetVertexBuffer(sphere[i]->Vbo, GetVertexLayoutPositionPNT());
      SetVertexBuffer(sphere[i]->Vbo, GetVertexLayoutNormalPNT());
      SetIndexBuffer(sphere[i]->Ibo);
      DrawIndexed(sphere[i]->IndexCount, 0);
    }
  }

  std::shared_ptr<Sphere> sphere[5];
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
    convDesc.TextureFormat = PixelFormat::RGB32F;
    convDesc.Width = 512;
    convDesc.Height = 512;
    for (int i = 0; i < 6; i++) {
      convDesc.DataFormat[i] = ImageDataFormat::RGB;
      convDesc.DataType[i] = ImageDataType::Float32;
      convDesc.DataPtr[i] = nullptr;
    }
    std::cout << "prefilter pillars_4k.hdr...";
    _conv = GetContext().PrefilterEnvMap(_skybox, convDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;

    LoadProgram("skybox.vert", "skybox.frag", {POSITION0()});
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
    SetVertexBuffer(*(_cube->Vbo), GetVertexLayoutPositionPNT());
    Draw(_cube->VertexCount, 0);
  }

 private:
  std::shared_ptr<TextureOpenGL> _skybox;
  std::shared_ptr<TextureOpenGL> _conv;
  std::shared_ptr<Cube> _cube;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Prefilter Environment Map"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ClearPass>();
  app.CreatePass<SkyboxPass>();
  app.CreatePass<ColorPass>();
  app.Instantiate<Cube>();
  for (int i = 0; i < 5; i++) {
    app.Instantiate<Sphere>(i, Vector3f{i - 2, 0, 0});
  }
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Awake();
  app.Run();
  return 0;
}