#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <type_traits>
#include <filesystem>
#include <any>
#include <chrono>

#include <hikari/mathematics.h>
#include <hikari/window.h>
#include <hikari/render_context.h>
#include <hikari/asset.h>
#include <hikari/camera.h>
#include <hikari/input.h>

namespace Hikari {

class Application;

struct Transform {
  Vector3f Position{0};
  Vector3f LocalScale{1};
  Matrix4f Rotation = Matrix4f::Identity();

  Quaternionf GetRotation() const;
  void SetRotation(const Quaternionf& q);

  Vector3f GetEulerAngle() const;
  void SetEulerAngle(const Vector3f& angle);

  Matrix4f ObjectToWorldMatrix() const;
};

class GameObject {
 public:
  GameObject() noexcept;
  GameObject(const std::string& name);
  virtual ~GameObject() noexcept;

  virtual void OnStart();
  virtual void OnUpdate();

  const std::string& GetName() const;
  void SetName(const std::string& name);
  Transform& GetTransform();
  const Transform& GetTransform() const;
  Application& GetApp();
  RenderContextOpenGL& GetContext();

  void CreateVbo(const ImmutableModel& model, std::shared_ptr<BufferOpenGL>& vbo);
  void CreateVboIbo(const ImmutableModel& model,
                    std::shared_ptr<BufferOpenGL>& vbo, std::shared_ptr<BufferOpenGL>& ibo);

 private:
  std::string _name;
  Transform _transform;
};

class MainCamera : public GameObject {
 public:
  MainCamera();
  ~MainCamera() override = default;
  void OnUpdate() override;

  bool CanOrbitCtrl{};
  std::unique_ptr<Camera> Camera;
  OrbitControls Orbit{};
};

class IRenderPass {
 public:
  virtual ~IRenderPass() noexcept = 0;
  virtual const std::string& GetName() const = 0;
  virtual int GetPriority() const = 0;
  virtual void OnStart() = 0;
  virtual void OnUpdate() = 0;
};

class RenderPass : public IRenderPass {
 public:
  RenderPass() noexcept;
  RenderPass(const std::string& name, int priority);
  ~RenderPass() noexcept override;
  const std::string& GetName() const override;
  int GetPriority() const override;
  void OnStart() override;
  void OnUpdate() override;

  Application& GetApp();
  std::shared_ptr<ProgramOpenGL>& GetProgram();
  RenderContextOpenGL& GetContext();
  Vector2i GetFrameBufferSize();
  int GetFBWidth();
  int GetFBHeight();
  std::unique_ptr<Camera>& GetCamera();
  void SetProgram(const std::shared_ptr<ProgramOpenGL>& prog);
  void SetProgram(const std::string& vs, const std::string& fs, const ShaderAttributeLayouts& layouts);
  PipelineState& GetPipelineState();
  void SetPipelineState(const PipelineState& state);
  void ActivePipelineConfig();
  void ActiveProgram();
  void SetVertexBuffer(const BufferOpenGL& vbo, const VertexBufferLayout& layout);
  void SetIndexBuffer(const BufferOpenGL& ibo);
  void SetVertexBuffer(const std::shared_ptr<BufferOpenGL>& vbo, const VertexBufferLayout& layout);
  void SetIndexBuffer(const std::shared_ptr<BufferOpenGL>& ibo);
  uint32_t BindTexture(const TextureOpenGL& texture);
  void Draw(int vertexCount, int vertexStart);
  void DrawIndexed(int indexCount, int indexStart);

 private:
  std::string _name;
  int _priority{};
  PipelineState _pipeState;
  std::shared_ptr<ProgramOpenGL> _prog;
  uint32_t _textureSlot{};
};

class AppRuntimeException : public std::runtime_error {
 public:
  explicit AppRuntimeException(const std::string& msg) noexcept : std::runtime_error(msg.c_str()) {}
  explicit AppRuntimeException(const char* msg) noexcept : std::runtime_error(msg) {}
};

class Application {
 public:
  Application(const Application&) = delete;
  Application(Application&&) = delete;
  ~Application();
  void SetWindowCreateInfo(const WindowCreateInfo& info);
  void SetRenderContextCreateInfo(const ContextOpenGLDescription& info);
  void Awake();
  void Run();

  NativeWindow& GetWindow();
  RenderContextOpenGL& GetContext();
  const Input& GetInput() const;
  MainCamera& GetCamera();
  const std::filesystem::path& GetAssetPath() const;
  const std::filesystem::path& GetShaderLibPath() const;
  std::shared_ptr<GameObject> GetGameObject(const std::string& name);
  std::any& GetSharedObject(const std::string& name);
  float GetDeltaTime();
  float GetTime();
  float GetFps();

  void AddPass(const std::shared_ptr<IRenderPass>& pass);
  void SetSharedObject(const std::string& name, std::any&& obj);
  void AddGameObject(const std::shared_ptr<GameObject>& gameObject);
  void SetCamera(std::unique_ptr<Camera>&& camera);
  void SetAssetPath(const std::filesystem::path&);
  void SetShaderLibPath(const std::filesystem::path&);
  void GetArgs(int argc, char** argv);

  template <class PassType, class... Args>
  void CreatePass(Args&&... args) {
    static_assert(std::is_base_of<IRenderPass, PassType>::value, "PassType must be a subclass of IRenderPass");
    AddPass(std::make_shared<PassType>(std::forward<Args>(args)...));
  }
  template <class GameObjectType, class... Args>
  void Instantiate(Args&&... args) {
    static_assert(std::is_base_of<GameObject, GameObjectType>::value, "GameObjectType must be a subclass of GameObject");
    AddGameObject(std::make_shared<GameObjectType>(std::forward<Args>(args)...));
  }
  template <class CameraType, class... Args>
  void CreateCamera(Args&&... args) {
    static_assert(std::is_base_of<Camera, CameraType>::value, "CameraType must be a subclass of Camera");
    SetCamera(std::make_unique<CameraType>(std::forward<Args>(args)...));
  }
  template <class ObjectType>
  std::shared_ptr<ObjectType> GetGameObject(const std::string& name) {
    static_assert(std::is_base_of<GameObject, ObjectType>::value, "ObjectType must be a subclass of GameObject");
    return std::dynamic_pointer_cast<ObjectType, GameObject>(GetGameObject(name));
  }
  template <class Type>
  Type GetSharedObject(const std::string& name) {
    auto& o = GetSharedObject(name);
    return std::any_cast<Type>(o);
  }

  static Application& GetInstance();

 private:
  Application();
  void UpdateTime();

  WindowCreateInfo _wdInfo;
  ContextOpenGLDescription _ctxDesc;
  std::filesystem::path _assetRoot;
  std::filesystem::path _shaderLibRoot;
  NativeWindow _window;
  RenderContextOpenGL _context;
  std::shared_ptr<MainCamera> _camera;
  Input _input;
  std::vector<std::shared_ptr<GameObject>> _gameObjects;
  std::unordered_map<std::string, std::shared_ptr<GameObject>> _gameObjectNameDict;
  std::vector<std::shared_ptr<IRenderPass>> _renderPasses;
  std::unordered_map<std::string, std::shared_ptr<IRenderPass>> _renderPassNameDict;
  std::unordered_map<std::string, std::any> _shared;
  int64_t _time{};
  int64_t _deltaTime{};
  std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime{};
  int64_t _frameCount{};
  int64_t _frameTimer{};
  float _fps{};
};

}  // namespace Hikari