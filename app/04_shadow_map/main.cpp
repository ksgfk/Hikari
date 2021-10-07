#include <iostream>
#include <filesystem>
#include <chrono>

#include <hikari/common.h>
#include <hikari/render_context.h>
#include <hikari/asset.h>
#include <hikari/camera.h>
#include <hikari/input.h>
#include <hikari/application.h>

using namespace std;
using namespace std::filesystem;
using namespace std::chrono;
using namespace Hikari;

const char shadowVert[] = R"glsl(
#version 330 core
in vec3 aPos;
uniform mat4 lightMVP;
void main() {
  gl_Position = lightMVP * vec4(aPos, 1.0);
}
)glsl";
const char shadowFrag[] = R"glsl(
#version 330 core
void main() {}
)glsl";

const char blinnVert[] = R"glsl(
#version 330 core
in vec3 aPos;
in vec3 aNormal;
out vec3 vPos;
out vec3 vNormal;
out vec4 vLightPos;
uniform mat4 model;
uniform mat4 invModel;
uniform mat4 mvp;
uniform mat4 lightMVP;
void main() {
  vPos = vec3(model * vec4(aPos, 1.0));
  vNormal = normalize(mat3(transpose(invModel)) * aNormal);
  vLightPos = lightMVP * vec4(aPos, 1.0);
  gl_Position = mvp * vec4(aPos, 1.0);
}
)glsl";
const char blinnFrag[] = R"glsl(
#version 330 core
#define BIAS 0.001
out vec4 FragColor;
in vec3 vPos;
in vec3 vNormal;
in vec4 vLightPos;
uniform vec3 _ambientColor;
uniform vec3 _diffuseColor;
uniform vec3 _lightDirection;
uniform vec3 _lightColor;
uniform vec3 _cameraPos;
uniform vec3 _specularColor;
uniform float _shininess;
uniform sampler2D _depth;
void main() {
  vec3 homoCrop = vLightPos.xyz / vLightPos.w;
  vec3 depthSpace = (homoCrop + 1) * 0.5;
  float depth = texture2D(_depth, depthSpace.xy).r;
  if(depth + BIAS < depthSpace.z) {
    FragColor = vec4(_ambientColor, 1.0);
    return;
  }

  vec3 result = _ambientColor;

  vec3 norm = normalize(vNormal);
  float nDotL = dot(norm, _lightDirection);
  if(nDotL > 0.0) {
    vec3 diffuse = max(nDotL, 0.0) * _lightColor * _diffuseColor;
    result += diffuse;

    vec3 viewDir = normalize(_cameraPos - vPos);
    vec3 halfDir = normalize(viewDir + _lightDirection);
    float spec = max(pow(dot(halfDir, norm), _shininess), 0);
    vec3 specular = _specularColor * _lightColor * spec;
    result += specular;
  }
  FragColor = vec4(result, 1.0f);
}
)glsl";

Vector3f lightPos = Vector3f(2.5f, 12.5f, 20.0f);
Vector3f lightTarget = Vector3f(0);
Vector3f lightUp = Vector3f(0, 1, 0);
Matrix4f lightLookAt = LookAt(lightPos, lightTarget, lightUp);
Matrix4f lightPersp = Ortho<float>(-20, 20, -20, 20, 0.1f, 100);
Vector3f lightDir = -Normalize(lightTarget - lightPos);

class Wall : public GameObject {
 public:
  Wall() : GameObject("Wall") {}
  void OnStart() override {
    ImmutableModel wall("wall", GetApp().GetAssetPath() / "04_wall.obj");
    CreateVboIbo(wall, Vbo, Ibo);
    IndexCount = int(wall.GetIndexCount());
  }

  std::shared_ptr<BufferOpenGL> Vbo;
  std::shared_ptr<BufferOpenGL> Ibo;
  int IndexCount{};
};

class Ring : public GameObject {
 public:
  Ring() : GameObject("Ring") {}
  void OnStart() override {
    ImmutableModel wall("ring", GetApp().GetAssetPath() / "04_ring.obj");
    CreateVboIbo(wall, Vbo, Ibo);
    IndexCount = int(wall.GetIndexCount());
  }
  void OnUpdate() override {
    Matrix4f old = GetTransform().Rotation;
    float deltaTime = GetApp().GetDeltaTime();
    Matrix4f rotateY = Rotate<float>({0, 1, 0}, Radian(45 * deltaTime));
    GetTransform().Rotation = rotateY * old;
  }
  std::shared_ptr<BufferOpenGL> Vbo;
  std::shared_ptr<BufferOpenGL> Ibo;
  int IndexCount{};
};

class DepthPass : public RenderPass {
 public:
  DepthPass() : RenderPass("Depth", 0) {}
  void OnStart() override {
    _wall = GetApp().GetGameObject<Wall>("Wall");
    _ring = GetApp().GetGameObject<Ring>("Ring");
    DepthTextureDescriptorOpenGL desc;
    desc.Wrap = WrapMode::Clamp;
    desc.MinFilter = FilterMode::Point;
    desc.MagFilter = FilterMode::Point;
    desc.Width = Resolve;
    desc.Height = Resolve;
    desc.TextureFormat = PixelFormat::Depth32;
    desc.DataType = ImageDataType::Float32;
    _depthTex = GetContext().CreateDepthTexture(desc);
    _depthFrame = GetContext().CreateFrameBuffer({_depthTex->GetHandle()});
    GetApp().SetSharedObject("depth_texture", _depthTex);
    SetPipelineState({});
    SetProgram(shadowVert, shadowFrag, {{"aPos", SemanticType::Vertex, 0}});
  }
  void OnUpdate() override {
    auto& prog = GetProgram();
    auto& ctx = GetContext();
    _depthFrame->Bind();
    ctx.SetViewport(0, 0, Resolve, Resolve);
    ctx.ClearDepth();
    ctx.ColorMask(false, false, false, false);
    ActivePipelineConfig();
    ActiveProgram();
    prog->UniformMat4("lightMVP", (_wall->GetTransform().ObjectToWorldMatrix() * lightLookAt * lightPersp).GetAddress());
    SetVertexBuffer(*(_wall->Vbo), GetVertexLayoutPositionPNT());
    SetIndexBuffer(*(_wall->Ibo));
    DrawIndexed(_wall->IndexCount, 0);
    prog->UniformMat4("lightMVP", (_ring->GetTransform().ObjectToWorldMatrix() * lightLookAt * lightPersp).GetAddress());
    SetVertexBuffer(*(_ring->Vbo), GetVertexLayoutPositionPNT());
    SetIndexBuffer(*(_ring->Ibo));
    DrawIndexed(_ring->IndexCount, 0);
    _depthFrame->Unbind();
  }

  int Resolve = 2048;

 private:
  std::shared_ptr<TextureOpenGL> _depthTex;
  std::shared_ptr<FrameBufferOpenGL> _depthFrame;
  std::shared_ptr<Wall> _wall;
  std::shared_ptr<Ring> _ring;
};

class BlinnPhongPass : public RenderPass {
 public:
  BlinnPhongPass() : RenderPass("Blinn Phong", 1) {}
  void OnStart() override {
    _wall = GetApp().GetGameObject<Wall>("Wall");
    _ring = GetApp().GetGameObject<Ring>("Ring");
    SetPipelineState({});
    SetProgram(blinnVert, blinnFrag, {{"aPos", SemanticType::Vertex, 0}, {"aNormal", SemanticType::Normal, 0}});
  }
  void OnUpdate() override {
    if (_depthTex == nullptr) {
      _depthTex = GetApp().GetSharedObject<std::shared_ptr<TextureOpenGL>>("depth_texture");
    }
    auto& blinnProg = GetProgram();
    auto& ctx = GetContext();
    auto view = GetCamera()->GetViewMatrix();
    auto proj = GetCamera()->GetProjectionMatrix();
    ctx.SetViewport(0, 0, GetFBWidth(), GetFBHeight());
    ctx.SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ctx.ColorMask(true, true, true, true);
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    blinnProg->UniformTexture2D("_depth", BindTexture(*_depthTex));
    auto wallModel = _wall->GetTransform().ObjectToWorldMatrix();
    blinnProg->UniformMat4("model", wallModel.GetAddress());
    blinnProg->UniformMat4("mvp", (wallModel * view * proj).GetAddress());
    Matrix4f invModel;
    Invert(wallModel, invModel);
    blinnProg->UniformMat4("invModel", invModel.GetAddress());
    blinnProg->UniformMat4("lightMVP", (wallModel * lightLookAt * lightPersp).GetAddress());
    blinnProg->UniformVec3("_ambientColor", Vector3f(0, 0, 0.2f).GetAddress());
    blinnProg->UniformVec3("_diffuseColor", Vector3f(0, 0, 1).GetAddress());
    blinnProg->UniformVec3("_lightDirection", lightDir.GetAddress());
    blinnProg->UniformVec3("_lightColor", Vector3f(1, 1, 1).GetAddress());
    blinnProg->UniformVec3("_cameraPos", GetCamera()->GetPosition().GetAddress());
    blinnProg->UniformVec3("_specularColor", Vector3f(1, 1, 1).GetAddress());
    blinnProg->UniformFloat("_shininess", 128.0f);
    SetVertexBuffer(*(_wall->Vbo), GetVertexLayoutPositionPNT());
    SetVertexBuffer(*(_wall->Vbo), GetVertexLayoutNormalPNT());
    SetIndexBuffer(*(_wall->Ibo));
    DrawIndexed(_wall->IndexCount, 0);

    auto ringModel = _ring->GetTransform().ObjectToWorldMatrix();
    blinnProg->UniformMat4("model", ringModel.GetAddress());
    blinnProg->UniformMat4("mvp", (ringModel * view * proj).GetAddress());
    Invert(ringModel, invModel);
    blinnProg->UniformMat4("invModel", invModel.GetAddress());
    blinnProg->UniformMat4("lightMVP", (ringModel * lightLookAt * lightPersp).GetAddress());
    blinnProg->UniformVec3("_diffuseColor", Vector3f(1, 0, 0).GetAddress());
    blinnProg->UniformVec3("_ambientColor", Vector3f(0.2f, 0, 0).GetAddress());
    SetVertexBuffer(*(_ring->Vbo), GetVertexLayoutPositionPNT());
    SetVertexBuffer(*(_ring->Vbo), GetVertexLayoutNormalPNT());
    SetIndexBuffer(*(_ring->Ibo));
    DrawIndexed(_ring->IndexCount, 0);
  }

 private:
  std::shared_ptr<TextureOpenGL> _depthTex;
  std::shared_ptr<Wall> _wall;
  std::shared_ptr<Ring> _ring;
};

int main(int argc, char** argv) {
  auto& app = Application::GetInstance();
  app.ParseArgs(argc, argv);
  app.SetWindowCreateInfo({"Hikari Shadow Map"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.GetCamera().Camera->SetPosition({0.0f, 0.0f, 40.0f});
  app.Instantiate<Wall>();
  app.Instantiate<Ring>();
  app.CreatePass<DepthPass>();
  app.CreatePass<BlinnPhongPass>();
  app.Awake();
  app.Run();
}