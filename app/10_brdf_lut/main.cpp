#include <iostream>

#include <hikari/application.h>

using namespace Hikari;

class Quad : public GameObject {
 public:
  Quad() : GameObject("quad") {}
  void OnStart() override {
    ImmutableModel cubeModel = ImmutableModel::CreateQuad("quad", 1);
    CreateVbo(cubeModel, Vbo);
    VertexCount = int(cubeModel.GetIndexCount());
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  int VertexCount{};
};

class ColorPass : public RenderPass {
 public:
  ColorPass() : RenderPass("Color Pass", 0) {}

  void OnStart() override {
    LoadProgram("texture.vert", "texture.frag", {POSITION(), TEXCOORD0()});

    Texture2dDescriptorOpenGL lutDesc;
    lutDesc.Wrap = WrapMode::Clamp;
    lutDesc.MinFilter = FilterMode::Bilinear;
    lutDesc.MagFilter = FilterMode::Bilinear;
    lutDesc.MipMapLevel = 0;
    lutDesc.TextureFormat = PixelFormat::RG16F;
    lutDesc.Width = 1024;
    lutDesc.Height = 1024;
    lutDesc.DataFormat = ImageDataFormat::RG;
    lutDesc.DataType = ImageDataType::Float32;
    std::cout << "precompute brdf lut...";
    lut = GetContext().PrecomputeBrdfLut(lutDesc, GetApp().GetShaderLibPath());
    std::cout << "Done." << std::endl;
  }

  void OnPostStart() override {
    quad = GetApp().GetGameObject<Quad>("quad");
  }

  void OnUpdate() override {
    GetContext().ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    SetViewportFullFrameBuffer();
    SetModelMatrix(*quad);
    GetProgram()->UniformTexture2D("u_texture", BindTexture(*lut));
    SetVertexBuffer(quad->Vbo, GetVertexPosPNT());
    SetVertexBuffer(quad->Vbo, GetVertexTexPNT(0));
    Draw(quad->VertexCount, 0);
  }

  std::shared_ptr<Quad> quad;
  std::shared_ptr<TextureOpenGL> lut;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari BRDF Lut"});
  app.SetRenderContextCreateInfo({3, 3});
  app.ParseArgs(argc, argv);
  app.CreatePass<ColorPass>();
  app.Instantiate<Quad>();
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.GetCamera().Camera->SetPosition({0, 0, 5});
  app.Awake();
  app.Run();
  return 0;
}