#include <hikari/application.h>

#include <iostream>
#include <algorithm>
#include <thread>
#include <cstring>

#include <hikari/opengl.h>

namespace Hikari {

static void __MessageCallbackOGL(GLenum source,
                                 GLenum type,
                                 GLuint id,
                                 GLenum severity,
                                 GLsizei length,
                                 GLchar const* message,
                                 void const* user_param) {
  auto const src_str = [](GLenum src) {
    switch (src) {
      case GL_DEBUG_SOURCE_API:
        return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "Window System";
      case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "Shader Compiler";
      case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "Third Party";
      case GL_DEBUG_SOURCE_APPLICATION:
        return "App";
      case GL_DEBUG_SOURCE_OTHER:
        return "Other";
      default:
        return "Unknown Source";
    }
  }(source);
  auto const type_str = [](GLenum tp) {
    switch (tp) {
      case GL_DEBUG_TYPE_ERROR:
        return "Error";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "Deprecated Behavior";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "Undefined Behavior";
      case GL_DEBUG_TYPE_PORTABILITY:
        return "Portability";
      case GL_DEBUG_TYPE_PERFORMANCE:
        return "Performance";
      case GL_DEBUG_TYPE_MARKER:
        return "Marker";
      case GL_DEBUG_TYPE_OTHER:
        return "Other";
      default:
        return "Unknown Type";
    }
  }(type);
  auto const severity_str = [](GLenum s) {
    switch (s) {
      case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "Notify";
      case GL_DEBUG_SEVERITY_LOW:
        return "Low";
      case GL_DEBUG_SEVERITY_MEDIUM:
        return "Medium";
      case GL_DEBUG_SEVERITY_HIGH:
        return "High";
      default:
        return "Unknown Severity";
    }
  }(severity);
  //去掉debug group的信息（
  if (source == GL_DEBUG_SOURCE_APPLICATION && severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    return;
  }
  std::cout << "[" << src_str << "," << type_str << "," << severity_str << "," << id << "]->" << message << std::endl;
}

Quaternionf Transform::GetRotation() const {
  return Quaternionf(Rotation);
}

void Transform::SetRotation(const Quaternionf& q) {
  Rotation = Rotate(q);
}

Vector3f Transform::GetEulerAngle() const {
  return EulerAngle(Rotation);
}

void Transform::SetEulerAngle(const Vector3f& angle) {
  Rotation = Rotate(angle);
}

Matrix4f Transform::ObjectToWorldMatrix() const {
  return Translate(Position) * Rotation * Scale(LocalScale);
}

GameObject::GameObject() noexcept = default;

GameObject::GameObject(const std::string& name) noexcept : GameObject() { _name = name; }

GameObject::~GameObject() noexcept = default;

void GameObject::OnStart() {}

void GameObject::OnUpdate() {}

const std::string& GameObject::GetName() const { return _name; }

void GameObject::SetName(const std::string& name) { _name = name; }

Transform& GameObject::GetTransform() { return _transform; }

const Transform& GameObject::GetTransform() const { return _transform; }

Application& GameObject::GetApp() { return Application::GetInstance(); }

RenderContextOpenGL& GameObject::GetContext() { return GetApp().GetContext(); }

void GameObject::CreateVbo(const ImmutableModel& model, std::shared_ptr<BufferOpenGL>& vbo) {
  auto d = GenVboDataPNT(model.GetPosition(), model.GetNormals(), model.GetTexCoords(), model.GetIndices());
  auto data = d.data();
  auto size = d.size() * sizeof(decltype(d)::value_type);
  vbo = GetApp().GetContext().CreateVertexBuffer(data, size);
}

void GameObject::CreateVboIbo(const ImmutableModel& model,
                              std::shared_ptr<BufferOpenGL>& vbo, std::shared_ptr<BufferOpenGL>& ibo) {
  auto vertex = GenVboDataPNT(model.GetPosition(), model.GetNormals(), model.GetTexCoords());
  auto vertexData = vertex.data();
  auto vertexSize = vertex.size() * sizeof(decltype(vertex)::value_type);
  vbo = GetApp().GetContext().CreateVertexBuffer(vertexData, vertexSize);
  std::vector<uint32_t> indices(model.GetIndexCount());
  for (size_t i = 0; i < model.GetIndexCount(); i++) {
    if (model.GetIndices()[i] >= std::numeric_limits<uint32_t>::max()) {
      throw AppRuntimeException("model vertex index out of range");
    }
    indices[i] = static_cast<decltype(indices)::value_type>(model.GetIndices()[i]);
  }
  auto indexData = indices.data();
  auto indexSize = indices.size() * sizeof(decltype(indices)::value_type);
  ibo = GetApp().GetContext().CreateIndexBuffer(indexData, indexSize);
}

MainCamera::MainCamera() : GameObject("Main Camera") {}

void MainCamera::OnUpdate() {
  int width, height;
  GetApp().GetWindow().GetFrameBufferSize(width, height);
  if (CanOrbitCtrl && Camera) {
    const auto& input = GetApp().GetInput();
    Orbit.Update(height,
                 input.GetCursePos(),
                 input.GetScrollDelta(),
                 input.GetMouse(MouseButton::Left),
                 input.GetMouse(MouseButton::Right));
    Orbit.ApplyCamera(Camera.get());
    auto& trans = GetTransform();
    trans.Position = Camera->GetPosition();
    trans.Rotation = Camera->GetSkyboxViewMatrix();
  }
  if (Camera) {
    Camera->SetAspect(width, height);
    SetGlobalCameraData();
  }
}

void MainCamera::SetGlobalCameraData() {
  auto& ctx = GetContext();
  auto v = Camera->GetViewMatrix();
  auto p = Camera->GetProjectionMatrix();
  Matrix4f invV;
  auto hasInv = Invert(v, invV);
  ctx.SetGlobalMat4(UNIFORM_VIEW_MATRIX, v);
  ctx.SetGlobalMat4(UNIFORM_PROJ_MATRIX, p);
  ctx.SetGlobalMat4(UNIFORM_VP_MATRIX, (v * p));
  if (hasInv) {
    ctx.SetGlobalMat4(UNIFORM_VIEW_MATRIX_INV, invV);
  } else {
    ctx.SetGlobalMat4(UNIFORM_VIEW_MATRIX_INV, Matrix4f::Identity());
  }
}

void MainCamera::SetCameraData(const ProgramOpenGL& prog) {
  prog.Bind();
  prog.UniformVec3(UNIFORM_CAMERA_POS, Camera->GetPosition().GetAddress());
}

Light::Light(const std::string& name) noexcept : GameObject(name) {}

Light::Light(const std::string& name,
             LightType type,
             const Vector3f& color,
             float intensity,
             const Vector3f& dir) noexcept
    : GameObject(name), Type(type), Color(color), Intensity(intensity), Direction(dir) {}

Light::~Light() noexcept = default;

LightCollection::LightCollection() noexcept = default;

LightCollection::LightCollection(int maxLight) noexcept {
  _maxLight = maxLight;
  _dir.reserve(maxLight);
  _point.reserve(maxLight);
  _dirRadianceData.reserve(maxLight);
  _dirDirectionData.reserve(maxLight);
  _pointRadianceData.reserve(maxLight);
  _pointDirectionData.reserve(maxLight);
}

bool LightCollection::AddLight(const std::shared_ptr<Light>& light) {
  switch (light->Type) {
    case LightType::Directional:
      if (_dir.size() >= _maxLight) {
        return false;
      }
      _dir.emplace_back(light);
      _dirRadianceData.emplace_back(Vec3Align16{});
      _dirDirectionData.emplace_back(Vec3Align16{});
      break;
    case LightType::Point:
      if (_point.size() >= _maxLight) {
        return false;
      }
      _point.emplace_back(light);
      _pointRadianceData.emplace_back(Vec3Align16{});
      _pointDirectionData.emplace_back(Vec3Align16{});
      break;
    default:
      throw AppRuntimeException("unknown light type");
  }
  return true;
}

void LightCollection::CollectData() {
  for (size_t i = 0; i < _dir.size(); i++) {
    _dirRadianceData[i] = {_dir[i]->Color * Vector3f(_dir[i]->Intensity)};
    _dirDirectionData[i] = {Normalize(_dir[i]->Direction)};
  }
  for (size_t i = 0; i < _point.size(); i++) {
    _pointRadianceData[i] = {_point[i]->Color * Vector3f(_point[i]->Intensity)};
    _pointDirectionData[i] = {Normalize(_point[i]->Direction)};
  }
}

void LightCollection::SubmitData(RenderContextOpenGL& ctx) const {
  ctx.SetGlobalVec3Array(UNIFORM_LIGHT_DIR_RAD, _dirRadianceData.data(), int(_dirRadianceData.size()));
  ctx.SetGlobalVec3Array(UNIFORM_LIGHT_DIR_DIR, _dirDirectionData.data(), int(_dirDirectionData.size()));
  ctx.SetGlobalVec3Array(UNIFORM_LIGHT_POINT_RAD, _pointRadianceData.data(), int(_pointRadianceData.size()));
  ctx.SetGlobalVec3Array(UNIFORM_LIGHT_POINT_DIR, _pointDirectionData.data(), int(_pointDirectionData.size()));
  ctx.SetGlobalInt(UNIFORM_LIGHT_DIR_CNT, int(_dir.size()));
  ctx.SetGlobalInt(UNIFORM_LIGHT_POINT_CNT, int(_point.size()));
}

void LightCollection::Clear() {
  _dir.clear();
  _point.clear();
  _dirRadianceData.clear();
  _dirDirectionData.clear();
  _pointRadianceData.clear();
  _pointDirectionData.clear();
}

IRenderPass::~IRenderPass() noexcept = default;

RenderPass::RenderPass() noexcept = default;

RenderPass::RenderPass(const std::string& name, int priority) : _name(name), _priority(priority) {}

RenderPass::~RenderPass() noexcept = default;

const std::string& RenderPass::GetName() const { return _name; }

int RenderPass::GetPriority() const { return _priority; }

void RenderPass::OnStart() {}

void RenderPass::OnPostStart() {}

void RenderPass::OnUpdate() {}

ProgramOpenGL& RenderPass::GetPipelineProgram() { return *_prog; }

bool RenderPass::HasPipelineProgram() { return _prog != nullptr; }

Application& RenderPass::GetApp() { return Application::GetInstance(); }

std::shared_ptr<ProgramOpenGL>& RenderPass::GetProgram() { return _prog; }

RenderContextOpenGL& RenderPass::GetContext() { return GetApp().GetContext(); }

Vector2i RenderPass::GetFrameBufferSize() {
  Vector2i result;
  GetApp().GetWindow().GetFrameBufferSize(result.X(), result.Y());
  return result;
}

int RenderPass::GetFrameBufferWidth() { return GetFrameBufferSize().X(); }

int RenderPass::GetFrameBufferHeight() { return GetFrameBufferSize().Y(); }

void RenderPass::SetViewportFullFrameBuffer() {
  auto size = GetFrameBufferSize();
  GetContext().SetViewport(0, 0, size.X(), size.Y());
}

std::unique_ptr<Camera>& RenderPass::GetCamera() { return GetApp().GetCamera().Camera; }

void RenderPass::SetProgram(const std::shared_ptr<ProgramOpenGL>& prog) { _prog = prog; }

void RenderPass::SetProgram(const std::string& vs, const std::string& fs, const ShaderAttributeLayouts& layouts) {
  _prog = GetApp().GetContext().CreateShaderProgram(vs, fs, layouts);
}

void RenderPass::LoadProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath,
                             const ShaderAttributeLayouts& layouts) {
  std::filesystem::path resVsPath;
  if (std::filesystem::exists(vsPath)) {
    resVsPath = vsPath;
  } else {
    resVsPath = GetApp().GetShaderLibPath() / vsPath;
  }
  std::filesystem::path resFsPath;
  if (std::filesystem::exists(fsPath)) {
    resFsPath = fsPath;
  } else {
    resFsPath = GetApp().GetShaderLibPath() / fsPath;
  }
  ImmutableText vs("vs", resVsPath);
  ImmutableText fs("fs", resFsPath);
  auto& ctx = GetContext();
  std::string resVs;
  if (!ctx.PreprocessShader(ShaderType::Vertex, vs.GetText(), resVs)) {
    std::cerr << "preprocess " << vsPath << " error" << std::endl;
    throw RenderContextException("can't preprocess vertex shader");
  }
  std::string resFs;
  if (!ctx.PreprocessShader(ShaderType::Fragment, fs.GetText(), resFs)) {
    std::cerr << "preprocess " << fsPath << " error" << std::endl;
    throw RenderContextException("can't preprocess fragment shader");
  }
  _prog = ctx.CreateShaderProgram(resVs, resFs, layouts);
}

PipelineState& RenderPass::GetPipelineState() { return _pipeState; }

void RenderPass::SetPipelineState(const PipelineState& state) { _pipeState = state; }

void RenderPass::ActivePipelineConfig() {
  const auto& setting = _pipeState;
  //Depth State
  if (setting.Depth.IsEnableDepthTest) {
    HIKARI_CHECK_GL(glEnable(GL_DEPTH_TEST));
    HIKARI_CHECK_GL(glDepthFunc(MapComparison(setting.Depth.Comparison)));
  } else {
    HIKARI_CHECK_GL(glDisable(GL_DEPTH_TEST));
  }
  HIKARI_CHECK_GL(glDepthMask(setting.Depth.IsEnableDepthWrite));
  _textureSlot = 0;
}

void RenderPass::ActiveProgram() {
  const auto& vao = GetApp().GetContext().GetVertexArray(_prog);
  _prog->Bind();
  vao.Bind();
}

void RenderPass::SetVertexBuffer(const BufferOpenGL& vbo, const VertexBufferLayout& layout) {
  auto bp = _prog->GetBindingPoint(layout.Semantic);
  if (bp < 0) {
    return;
  }
  const auto& vao = GetApp().GetContext().GetVertexArray(_prog);
  vao.SetVertexBuffer({(GLuint)bp, vbo.GetHandle(), layout.Offset, layout.Stride});
}

void RenderPass::SetIndexBuffer(const BufferOpenGL& ibo) {
  const auto& vao = GetApp().GetContext().GetVertexArray(_prog);
  vao.SetIndexBuffer(ibo.GetHandle());
}

void RenderPass::SetVertexBuffer(const std::shared_ptr<BufferOpenGL>& vbo, const VertexBufferLayout& layout) {
  SetVertexBuffer(*vbo, layout);
}

void RenderPass::SetIndexBuffer(const std::shared_ptr<BufferOpenGL>& ibo) {
  SetIndexBuffer(*ibo);
}

uint32_t RenderPass::BindTexture(const TextureOpenGL& texture) {
  auto slot = _textureSlot;
  HIKARI_CHECK_GL(glActiveTexture(GL_TEXTURE0 + slot));
  auto type = TextureOpenGL::MapTextureType(texture.GetType());
  HIKARI_CHECK_GL(glBindTexture(type, texture.GetHandle()));
  _textureSlot++;
  return slot;
}

void RenderPass::SetModelMatrix(const GameObject& go) {
  auto m = go.GetTransform().ObjectToWorldMatrix();
  Matrix4f invM;
  auto hasInv = Invert(m, invM);
  _prog->UniformMat4(UNIFORM_MODEL_MATRIX, m.GetAddress());
  if (hasInv) {
    _prog->UniformMat4(UNIFORM_MODEL_MATRIX_INV, invM.GetAddress());
  } else {
    _prog->UniformMat4(UNIFORM_MODEL_MATRIX_INV, Matrix4f::Identity().GetAddress());
  }
}

void RenderPass::Draw(int vertexCount, int vertexStart) {
  GetApp().GetContext().DrawArrays(_pipeState.Primitive, vertexStart, vertexCount);
}

void RenderPass::DrawIndexed(int indexCount, int indexStart) {
  GetApp().GetContext().DrawElements(_pipeState.Primitive, indexCount, IndexDataType::UnsignedInt, indexStart);
}

Application::Application() {
  glfwSetErrorCallback([](int error, const char* descr) {
    std::cerr << "glfw error " << error << ':' << descr << std::endl;
  });
  if (!glfwInit()) {
    throw AppRuntimeException("can't init glfw");
  }
  std::cout << "glfw successfully initialized" << std::endl;
  _camera = std::make_shared<MainCamera>();
  AddGameObject(_camera);
}

Application::~Application() {
  std::cout << "application terminate" << std::endl;
}

void Application::SetWindowCreateInfo(const WindowCreateInfo& info) { _wdInfo = info; }

void Application::SetRenderContextCreateInfo(const ContextOpenGLDescription& info) { _ctxDesc = info; }

void Application::Awake() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _ctxDesc.MajorVersion);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _ctxDesc.MinorVersion);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(NDEBUG)
  glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  auto glfwWindow = glfwCreateWindow(_wdInfo.Width, _wdInfo.Height, _wdInfo.Title.c_str(), nullptr, nullptr);
  if (glfwWindow == nullptr) {
    throw AppRuntimeException("can't create glfw window");
  }
  glfwMakeContextCurrent(glfwWindow);
  glfwSetFramebufferSizeCallback(
      glfwWindow, [](auto window, auto width, auto height) -> auto {
        auto& app = Application::GetInstance();
        const auto& p = app._window._resizeEvent;
        for (const auto& e : p) {
          if (e) {
            e(width, height);
          }
        }
      });
  _window._handle = glfwWindow;
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw AppRuntimeException("can't init opengl");
  }
  auto& feature = FeatureOpenGL::Get();
  feature.Init();
  HIKARI_CHECK_GL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
#if !defined(NDEBUG)
  if (feature.CanEnableDebug()) {
    HIKARI_CHECK_GL(glEnable(GL_DEBUG_OUTPUT));
    HIKARI_CHECK_GL(glDebugMessageCallback(__MessageCallbackOGL, nullptr));
  }
#endif
  std::cout << "opengl context loaded" << std::endl;
  std::cout << "renderer:" << feature.GetHardwareInfo() << std::endl;
  std::cout << "driver:" << feature.GetDriverInfo() << std::endl;
  if (_shaderLibRoot.empty()) {
    _shaderLibRoot = _assetRoot / "shaders";
  }
  _context.Init(_shaderLibRoot);
  if (_camera->Camera == nullptr) {
    _camera->Camera = std::make_unique<PerspectiveCamera>();  //假装是透视相机
  }
  for (auto& gameObject : _gameObjects) {
    gameObject->OnStart();
  }
  for (auto& renderPass : _renderPasses) {
    renderPass->OnStart();
  }
  for (auto& renderPass : _renderPasses) {
    renderPass->OnPostStart();
  }
  if (feature.GetMajorVersion() >= 4 && feature.GetMinorVersion() >= 1) {
    HIKARI_CHECK_GL(glReleaseShaderCompiler());
  }
}

void Application::Run() {
  const auto& feature = FeatureOpenGL::Get();
  while (!_window.ShouldClose()) {
    _input.Update(_window.GetHandle());      //更新input输入
    for (auto& gameObject : _gameObjects) {  //更新GameObject
      gameObject->OnUpdate();
    }
    _lights.CollectData();
    _lights.SubmitData(_context);             //上传灯光
    _context.SubmitGlobalUnifroms();          //上传所有uniform block块数据
    for (auto& renderPass : _renderPasses) {  //更新pass
      if (renderPass->HasPipelineProgram()) {
        _camera->SetCameraData(renderPass->GetPipelineProgram());  //更新uniform相机数据
      }
      if (feature.GetMajorVersion() >= 4 && feature.GetMinorVersion() >= 3) {
        HIKARI_CHECK_GL(glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, renderPass->GetName().c_str()));
        renderPass->OnUpdate();
        HIKARI_CHECK_GL(glPopDebugGroup());
      } else {
        renderPass->OnUpdate();
      }
    }
    _window.PollEvents();
    _window.SwapBuffers();
    UpdateTime();
    std::this_thread::yield();
  }
  Destroy();
}

void Application::Destroy() {
  _camera = nullptr;
  _lights.Clear();
  _gameObjects.clear();
  _renderPasses.clear();
  _context.Destroy();
  _window.Destroy();  //不知道为啥ubuntu在析构函数里销毁窗口就报错...make ubuntu happy :(
}

void Application::AddPass(const std::shared_ptr<IRenderPass>& pass) {
  if (_renderPassNameDict.find(pass->GetName()) != _renderPassNameDict.end()) {
    throw AppRuntimeException("render pass name must be unique");
  }
  auto p = std::lower_bound(_renderPasses.begin(), _renderPasses.end(),
                            pass->GetPriority(),
                            [](const auto& l, const auto& r) {
                              return (l->GetPriority()) < r;
                            });
  auto idx = std::distance(_renderPasses.begin(), p);
  _renderPasses.emplace(p, pass);
  _renderPassNameDict[pass->GetName()] = pass;
}

void Application::AddGameObject(const std::shared_ptr<GameObject>& gameObject) {
  if (gameObject->GetName().empty()) {
    throw AppRuntimeException("game object name cannot empty");
  }
  if (_gameObjectNameDict.find(gameObject->GetName()) != _gameObjectNameDict.end()) {
    throw AppRuntimeException("game object name must be unique");
  }
  auto idx = std::distance(_gameObjects.begin(), _gameObjects.end());
  _gameObjects.emplace_back(gameObject);
  _gameObjectNameDict[gameObject->GetName()] = gameObject;
}

void Application::AddLight(const std::shared_ptr<Light>& light) {
  if (!_lights.AddLight(light)) {
    std::cerr << "max light count" << std::endl;
  } else {
    AddGameObject(light);
  }
}

void Application::SetCamera(std::unique_ptr<Camera>&& camera) { _camera->Camera = std::move(camera); }

std::shared_ptr<GameObject> Application::GetGameObject(const std::string& name) {
  auto iter = _gameObjectNameDict.find(name);
  if (iter == _gameObjectNameDict.end()) {
    throw AppRuntimeException(std::string("can't find GameObject ") + name);
  }
  return iter->second;
}

NativeWindow& Application::GetWindow() { return _window; }

RenderContextOpenGL& Application::GetContext() { return _context; }

const Input& Application::GetInput() const { return _input; }

MainCamera& Application::GetCamera() { return *_camera; }

void Application::SetAssetPath(const std::filesystem::path& root) { _assetRoot = root; }

void Application::SetShaderLibPath(const std::filesystem::path& root) { _shaderLibRoot = root; }

void Application::ParseArgs(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "Command-line arguments options:\n"
              << "  -A | --asset    Set asset root path\n"
              << "  --shader-lib    Set shader library root path.Default location is \"shaders\" folder in the asset path\n"
              << std::endl;
  }
  for (int i = 1; i < argc;) {
    if (strncmp(argv[i], "--asset", 7) == 0 || strncmp(argv[i], "-A", 2) == 0) {
      if (i == argc - 1) {
        throw AppRuntimeException("invalid argument.--asset/-A must follow asset path");
      }
      SetAssetPath(argv[i + 1]);
      i += 2;
    } else if (strncmp(argv[i], "--shader-lib", 12) == 0) {
      SetShaderLibPath(argv[i + 1]);
      i += 2;
    } else {
      std::cout << "unknown argument " << argv[i] << std::endl;
      i++;
    }
  }
  if (_assetRoot.empty()) {
    std::string rootPath;
    std::cout << "Please input asset path:";
    std::cin >> rootPath;
    SetAssetPath(rootPath);
  }
}

const std::filesystem::path& Application::GetAssetPath() const { return _assetRoot; }

const std::filesystem::path& Application::GetShaderLibPath() const { return _shaderLibRoot; }

std::any& Application::GetSharedObject(const std::string& name) {
  auto iter = _shared.find(name);
  if (iter == _shared.end()) {
    throw AppRuntimeException(std::string("can't find shared object ") + name);
  }
  return iter->second;
}

void Application::SetSharedObject(const std::string& name, std::any&& obj) {
  _shared[name] = std::move(obj);
}

float Application::GetDeltaTime() { return _deltaTime / 1000.0f; }

float Application::GetTime() { return _time / 1000.0f; }

float Application::GetFps() { return _fps; }

Application& Application::GetInstance() {
  static Application app;
  return app;
}

void Application::UpdateTime() {
  auto now = std::chrono::high_resolution_clock::now();
  _deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - _lastFrameTime).count();
  _lastFrameTime = now;
  _frameCount++;
  if (_frameTimer < 1000) {
    _frameTimer += _deltaTime;
  } else {
    _fps = float(_frameCount) / (_frameTimer / 1000.0f);
    _frameTimer = 0;
    _frameCount = 0;
  }
}

}  // namespace Hikari