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

#include <imgui.h>

namespace Hikari {

class Application;
class RenderPass;

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
  GameObject(const std::string& name) noexcept;
  virtual ~GameObject() noexcept;

  virtual void OnStart();
  virtual void OnPostStart();
  virtual void OnUpdate();
  virtual void OnGui();

  const std::string& GetName() const;
  void SetName(const std::string& name);
  Transform& GetTransform();
  const Transform& GetTransform() const;
  Application& GetApp();
  RenderContextOpenGL& GetContext();

  static void CreateVbo(const ImmutableModel& model, std::shared_ptr<BufferOpenGL>& vbo);
  static void CreateVboIbo(
      const ImmutableModel& model,
      std::shared_ptr<BufferOpenGL>& vbo,
      std::shared_ptr<BufferOpenGL>& ibo);

 private:
  std::string _name;
  Transform _transform;
};

//嗯...GetTransform()只能用来获取位置啊之类的数据，在Transform里设置不会起效果...（历史遗留问题
class MainCamera : public GameObject {
 public:
  MainCamera();
  ~MainCamera() override = default;
  void OnUpdate() override;

  void SetGlobalCameraData();
  void SetCameraData(const ProgramOpenGL& prog);

  bool CanOrbitCtrl{};
  std::unique_ptr<Camera> Camera;
  OrbitControls Orbit{};
};

enum class LightType {
  Directional,
  Point
};

class Light : public GameObject {
 public:
  Light(const std::string& name) noexcept;
  Light(const std::string& name, LightType type, const Vector3f& color, float intensity, const Vector3f& dir) noexcept;
  ~Light() noexcept override;

  LightType Type{LightType::Directional};
  Vector3f Color{1, 1, 1};
  float Intensity{1};
  Vector3f Direction{0, -1, 0};
};

class LightCollection {
 public:
  struct alignas(16) Vec3Align16 {
    Vector3f Data;
  };

  LightCollection() noexcept;
  LightCollection(int maxLight) noexcept;

  bool AddLight(const std::shared_ptr<Light>& light);
  void CollectData();
  void SubmitData(RenderContextOpenGL& ctx) const;
  void Clear();
  std::vector<std::string> GetMacro() const;

 private:
  int _maxLight = 4096;
  std::vector<std::shared_ptr<Light>> _dir;
  std::vector<std::shared_ptr<Light>> _point;
  std::vector<Vec3Align16> _dirRadianceData;
  std::vector<Vec3Align16> _dirDirectionData;
  std::vector<Vec3Align16> _pointRadianceData;
  std::vector<Vec3Align16> _pointDirectionData;
};

//其实是GPU Buffer管理类（
class Renderable {
 public:
  Renderable() noexcept;
  virtual ~Renderable();

  virtual void OnCreate() = 0;

  const std::shared_ptr<BufferOpenGL>& GetVbo() const { return _vbo; }
  const std::shared_ptr<BufferOpenGL>& GetIbo() const { return _ibo; }
  int GetDrawCount() const { return _drawCount; }
  bool HasIbo() const;
  void Draw(RenderPass& pass) const;

 protected:
  void CreateVbo(const ImmutableModel& model);
  void CreateVboIbo(const ImmutableModel& model);
  void CreateVboWithTangent(const ImmutableModel& model);
  void CreateVboIboWithTangent(const ImmutableModel& model);

 private:
  std::shared_ptr<BufferOpenGL> _vbo;
  std::shared_ptr<BufferOpenGL> _ibo;
  int _drawCount{};
};

class RenderableWithTangent : public Renderable {
 public:
  RenderableWithTangent(float hasTan) noexcept;
  ~RenderableWithTangent() override = default;

 protected:
  bool HasTangent() const { return _hasTangent; }

 private:
  bool _hasTangent;
};

class RenderableCube : public RenderableWithTangent {
 public:
  RenderableCube(float halfExtend, bool getTan = false) noexcept;
  ~RenderableCube() override = default;

  void OnCreate() override;

 private:
  float _halfExtend;
};

class RenderableSphere : public RenderableWithTangent {
 public:
  RenderableSphere(float radius, int numberSlices, bool getTan = false) noexcept;
  ~RenderableSphere() override = default;

  void OnCreate() override;

 private:
  float _radius;
  int _slice;
};

class RenderableQuad : public RenderableWithTangent {
 public:
  RenderableQuad(float halfExtend, bool getTan = false, float offset = 0) noexcept;
  ~RenderableQuad() override = default;

  void OnCreate() override;

 private:
  float _halfExtend;
  bool _offset;
};

class RenderableSimple : public RenderableWithTangent {
 public:
  RenderableSimple(std::function<ImmutableModel(Renderable&)> onCreate, bool getTan = false);
  ~RenderableSimple() override = default;

  void OnCreate() final;

 private:
  std::function<ImmutableModel(Renderable&)> _onCreate;
};

class IRenderPass {
 public:
  virtual ~IRenderPass() noexcept = 0;
  virtual const std::string& GetName() const = 0;
  virtual int GetPriority() const = 0;
  virtual void OnStart() = 0;
  virtual void OnPostStart() = 0;
  virtual void OnUpdate() = 0;
  virtual ProgramOpenGL& GetPipelineProgram() = 0;
  virtual bool HasPipelineProgram() = 0;
};

class RenderPass : public IRenderPass {
 public:
  RenderPass() noexcept;
  RenderPass(const std::string& name, int priority);
  ~RenderPass() noexcept override;
  const std::string& GetName() const override;
  int GetPriority() const override;
  void OnStart() override;
  void OnPostStart() override;
  void OnUpdate() override;
  ProgramOpenGL& GetPipelineProgram() override;
  bool HasPipelineProgram() override;

  Application& GetApp();
  std::shared_ptr<ProgramOpenGL>& GetProgram();
  RenderContextOpenGL& GetContext();
  Vector2i GetFrameBufferSize();
  int GetFrameBufferWidth();
  int GetFrameBufferHeight();
  void SetViewportFullFrameBuffer();
  std::unique_ptr<Camera>& GetCamera();
  void SetProgram(const std::shared_ptr<ProgramOpenGL>& prog);
  void SetProgram(const std::string& vs, const std::string& fs, const ShaderAttributeLayouts& layouts);
  void LoadProgram(const std::filesystem::path& vsPath,
                   const std::filesystem::path& fsPath,
                   const ShaderAttributeLayouts& layouts);
  PipelineState& GetPipelineState();
  void SetPipelineState(const PipelineState& state);

  void ActivePipelineConfig();
  void ActiveProgram();
  void SetVertexBuffer(const BufferOpenGL& vbo, const VertexBufferLayout& layout);
  void SetIndexBuffer(const BufferOpenGL& ibo);
  void SetVertexBuffer(const std::shared_ptr<BufferOpenGL>& vbo, const VertexBufferLayout& layout);
  void SetIndexBuffer(const std::shared_ptr<BufferOpenGL>& ibo);
  uint32_t BindTexture(const TextureOpenGL& texture);
  void SetModelMatrix(const GameObject& go);
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

//上传light数据
class Application {
 public:
  Application(const Application&) = delete;
  Application(Application&&) = delete;
  ~Application();
  void SetWindowCreateInfo(const WindowCreateInfo& info);
  void SetRenderContextCreateInfo(const ContextOpenGLDescription& info);
  void Awake();
  void Run();
  void Destroy();

  NativeWindow& GetWindow();
  RenderContextOpenGL& GetContext();
  const Input& GetInput() const;
  MainCamera& GetCamera();
  const std::filesystem::path& GetAssetPath() const;
  const std::filesystem::path& GetShaderLibPath() const;
  std::shared_ptr<GameObject> GetGameObject(const std::string& name);
  std::shared_ptr<IRenderPass> GetRenderPass(const std::string& name);
  std::any& GetSharedObject(const std::string& name);
  bool HasSharedObject(const std::string& name);
  std::shared_ptr<Renderable> GetRenderable(const std::string& name);
  float GetDeltaTime();
  float GetTime();
  float GetFps();
  int64_t GetRealTime();
  LightCollection& GetLights() { return _lights; }

  void AddPass(const std::shared_ptr<IRenderPass>& pass);
  void SetSharedObject(const std::string& name, std::any&& obj);
  void AddGameObject(const std::shared_ptr<GameObject>& gameObject);
  void AddLight(const std::shared_ptr<Light>& light);
  void SetCamera(std::unique_ptr<Camera>&& camera);
  void SetAssetPath(const std::filesystem::path&);
  void SetShaderLibPath(const std::filesystem::path&);
  void ParseArgs(int argc, char** argv);
  void AddRenderable(const std::string& name, const std::shared_ptr<Renderable>& renderable);
  void EnableImgui();

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
  template <class LightType, class... Args>
  void CreateLight(Args&&... args) {
    static_assert(std::is_base_of<Light, LightType>::value, "LightType must be a subclass of Light");
    AddLight(std::make_shared<LightType>(std::forward<Args>(args)...));
  }
  template <class CameraType, class... Args>
  void CreateCamera(Args&&... args) {
    static_assert(std::is_base_of<Camera, CameraType>::value, "CameraType must be a subclass of Camera");
    SetCamera(std::make_unique<CameraType>(std::forward<Args>(args)...));
  }
  template <class RenderableType, class... Args>
  void CreateRenderable(const std::string& name, Args&&... args) {
    static_assert(std::is_base_of<Renderable, RenderableType>::value, "RenderableType must be a subclass of Renderable");
    AddRenderable(name, std::make_shared<RenderableType>(std::forward<Args>(args)...));
  }
  template <class ObjectType>
  std::shared_ptr<ObjectType> GetGameObject(const std::string& name) {
    static_assert(std::is_base_of<GameObject, ObjectType>::value, "ObjectType must be a subclass of GameObject");
    return std::dynamic_pointer_cast<ObjectType, GameObject>(GetGameObject(name));
  }
  template <class PassType>
  std::shared_ptr<PassType> GetRenderPass(const std::string& name) {
    static_assert(std::is_base_of<IRenderPass, PassType>::value, "PassType must be a subclass of IRenderPass");
    return std::dynamic_pointer_cast<PassType, IRenderPass>(GetRenderPass(name));
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
  LightCollection _lights;
  std::vector<std::shared_ptr<IRenderPass>> _renderPasses;
  std::unordered_map<std::string, std::shared_ptr<IRenderPass>> _renderPassNameDict;
  std::unordered_map<std::string, std::any> _shared;
  std::unordered_map<std::string, std::shared_ptr<Renderable>> _renderables;
  int64_t _time{};
  int64_t _deltaTime{};
  std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime{};
  int64_t _frameCount{};
  int64_t _frameTimer{};
  float _fps{};
  bool _canUseImgui{};
};

}  // namespace Hikari