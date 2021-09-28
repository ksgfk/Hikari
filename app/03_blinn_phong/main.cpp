#include <iostream>

#include <hikari/application.h>

using namespace Hikari;

const char blinnVs[] = R"glsl(
#version 330 core
in vec3 aPos;
in vec3 aNormal;
out vec3 vPos;
out vec3 vNormal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main() {
  vPos = vec3(model * vec4(aPos, 1.0));
  vNormal = normalize(mat3(transpose(model)) * aNormal);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";
const char blinnFs[] = R"glsl(
#version 330 core
out vec4 FragColor;
in vec3 vPos;
in vec3 vNormal;
uniform vec3 _ambientColor;
uniform vec3 _diffuseColor;
uniform vec3 _lightDirection;
uniform vec3 _lightColor;
uniform vec3 _cameraPos;
uniform vec3 _specularColor;
uniform float _shininess;
void main() {
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

class BlinnPhongPass : public RenderPass {
 public:
  BlinnPhongPass() : RenderPass("Blinn Phong", 0) {}
  void OnStart() override {
    _sphere = GetApp().GetGameObject<Sphere>("Sphere");
    SetPipelineState({});
    SetProgram(blinnVs, blinnFs, {{"aPos", SemanticType::Vertex, 0}, {"aNormal", SemanticType::Normal, 0}});
  }
  void OnUpdate() override {
    auto& blinn = GetProgram();
    auto& ctx = GetContext();
    ctx.SetViewport(0, 0, GetFBWidth(), GetFBHeight());
    ctx.SetClearColor(0, 0, 0, 0);
    ctx.ClearColorAndDepth();
    ActivePipelineConfig();
    ActiveProgram();
    blinn->UniformMat4("model", _sphere->GetTransform().ObjectToWorldMatrix().GetAddress());
    blinn->UniformMat4("view", GetCamera()->GetViewMatrix().GetAddress());
    blinn->UniformMat4("projection", GetCamera()->GetProjectionMatrix().GetAddress());
    blinn->UniformVec3("_ambientColor", Vector3f(0, 0, 0.2f).GetAddress());
    blinn->UniformVec3("_diffuseColor", Vector3f(0, 0, 1).GetAddress());
    blinn->UniformVec3("_lightDirection", Vector3f(1, 0, 0).GetAddress());
    blinn->UniformVec3("_lightColor", Vector3f(1, 1, 1).GetAddress());
    blinn->UniformVec3("_cameraPos", GetCamera()->GetPosition().GetAddress());
    blinn->UniformVec3("_specularColor", Vector3f(1, 1, 1).GetAddress());
    blinn->Uniform("_shininess", 32.0f);
    SetVertexBuffer(*(_sphere->Vbo), GetVertexLayoutPositionPNT());
    SetVertexBuffer(*(_sphere->Vbo), GetVertexLayoutNormalPNT());
    SetIndexBuffer(*(_sphere->Ibo));
    DrawIndexed(_sphere->IndexCount, 0);
  }

 private:
  std::shared_ptr<Sphere> _sphere;
};

int main() {
  auto& app = Application::GetInstance();
  app.SetWindowCreateInfo({"Hikari Blinn-Phong"});
  app.SetRenderContextCreateInfo({3, 3});
  app.CreateCamera<PerspectiveCamera>();
  app.GetCamera().CanOrbitCtrl = true;
  app.Instantiate<Sphere>();
  app.CreatePass<BlinnPhongPass>();
  app.Awake();
  app.Run();
}