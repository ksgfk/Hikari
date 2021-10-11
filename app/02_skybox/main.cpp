#include <iostream>

#include <hikari/application.h>

using namespace Hikari;
using namespace std;

const char skyVs[] = R"glsl(
#version 330 core
in vec3 aPos;
out vec3 vTexCoords;
uniform mat4 projection;
uniform mat4 view;
void main() {
  vTexCoords = aPos;
  vec4 pos = projection * view * vec4(aPos, 1.0);
  gl_Position = pos.xyww;
}
)glsl";
const char skyFs[] = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 vTexCoords;
uniform samplerCube skybox;
void main() {    
  FragColor = texture(skybox, vTexCoords);
}
)glsl";

const char sphereVs[] = R"glsl(
#version 330 core
in vec3 aPosition;
in vec3 aNormal;
in vec3 aTexCoord;
out vec3 vNormal;//flat?
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main() {
  mat4 normalMatrix = transpose(inverse(model));
  vNormal = normalize(vec3(normalMatrix * vec4(aNormal, 1.0)));
  gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
)glsl";
const char sphereFs[] = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 vNormal;
void main() {
  FragColor = vec4((vNormal + 1.0f) * 0.5f, 1.0);
}
)glsl";

class Cube : public GameObject {
 public:
  Cube() : GameObject("Cube") {}
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
  Sphere() : GameObject("Sphere") {}
  void OnStart() override {
    ImmutableModel sphereModel = ImmutableModel::CreateSphere("sphere", 1, 32);
    CreateVboIbo(sphereModel, Vbo, Ibo);
    IndexCount = int(sphereModel.GetIndexCount());
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  std::shared_ptr<BufferOpenGL> Ibo;
  int IndexCount{};
};

class SkyboxPass : public RenderPass {
 public:
  SkyboxPass() : RenderPass("Skybox", 1) {}
  void OnStart() override {
    ImmutableBitmap bitmap[6];
    for (size_t i = 0; i < 6; i++) {
      bitmap[i] = ImmutableBitmap(faces[i], GetApp().GetAssetPath() / "skybox1" / faces[i], false);
      if (!bitmap[i].IsValid()) {
        throw AppRuntimeException(std::string("Can't load bitmap ") + faces[i]);
      }
    }
    TextureCubeMapDescriptorOpenGL cubeDesc{};
    cubeDesc.Wrap = WrapMode::Clamp;
    cubeDesc.MinFilter = FilterMode::Bilinear;
    cubeDesc.MagFilter = FilterMode::Bilinear;
    cubeDesc.TextureFormat = PixelFormat::RGB8;
    cubeDesc.Width = 2048;
    cubeDesc.Height = 2048;
    for (size_t i = 0; i < 6; i++) {
      cubeDesc.DataFormat[i] = bitmap[i].GetChannel() == 3 ? ColorFormat::RGB : ColorFormat::RGBA;
      cubeDesc.DataType[i] = ImageDataType::Byte;
      cubeDesc.DataPtr[i] = (void*)bitmap[i].GetData();
    }
    _skybox = GetContext().CreateCubeMap(cubeDesc);
    _cube = GetApp().GetGameObject<Cube>("Cube");
    SetProgram(skyVs, skyFs, {{"aPos", SemanticType::Vertex, 0}});
    PipelineState state{};
    state.Depth.Comparison = DepthComparison::LessEqual;
    SetPipelineState(state);
  }
  void OnUpdate() override {
    auto& prog = GetProgram();
    auto& ctx = GetContext();
    ActivePipelineConfig();
    ActiveProgram();
    prog->UniformMat4("view", GetCamera()->GetSkyboxViewMatrix().GetAddress());
    prog->UniformMat4("projection", GetCamera()->GetProjectionMatrix().GetAddress());
    prog->UniformCubeMap("skybox", BindTexture(*_skybox));
    SetVertexBuffer(*(_cube->Vbo), GetVertexLayoutPositionPNT());
    Draw(_cube->VertexCount, 0);
  }

 private:
  std::shared_ptr<TextureOpenGL> _skybox;
  std::shared_ptr<Cube> _cube;
  const vector<std::string> faces{
      "right.jpg",
      "left.jpg",
      "top.jpg",
      "bottom.jpg",
      "front.jpg",
      "back.jpg"};
};

class NormalPass : public RenderPass {
 public:
  NormalPass() : RenderPass("Draw Normal", 0) {}
  void OnStart() override {
    _sphere = GetApp().GetGameObject<Sphere>("Sphere");
    SetProgram(sphereVs, sphereFs, {{"aPosition", SemanticType::Vertex, 0}, {"aNormal", SemanticType::Normal, 0}});
    SetPipelineState({});
  }
  void OnUpdate() override {
    auto& prog = GetProgram();
    auto& ctx = GetContext();
    ctx.SetViewport(0, 0, GetFrameBufferWidth(), GetFrameBufferHeight());
    ctx.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    prog->UniformMat4("model", _sphere->GetTransform().ObjectToWorldMatrix().GetAddress());
    prog->UniformMat4("view", GetCamera()->GetViewMatrix().GetAddress());
    prog->UniformMat4("projection", GetCamera()->GetProjectionMatrix().GetAddress());
    SetVertexBuffer(*(_sphere->Vbo), GetVertexLayoutPositionPNT());
    SetVertexBuffer(*(_sphere->Vbo), GetVertexLayoutNormalPNT());
    SetIndexBuffer(*(_sphere->Ibo));
    DrawIndexed(_sphere->IndexCount, 0);
  }

 private:
  std::shared_ptr<Sphere> _sphere;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.ParseArgs(argc, argv);
  app.SetWindowCreateInfo({"Hikari Skybox"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Instantiate<Cube>();
  app.Instantiate<Sphere>();
  app.CreatePass<SkyboxPass>();
  app.CreatePass<NormalPass>();
  app.Awake();
  app.Run();
}