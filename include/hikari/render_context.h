#pragma once

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <optional>

#include <hikari/mathematics.h>
#include <hikari/opengl.h>
#include <hikari/asset.h>

namespace Hikari {
class RenderPass;
class RenderContextOpenGL;

class RenderContextException : public std::runtime_error {
 public:
  explicit RenderContextException(const std::string& msg) noexcept : std::runtime_error(msg.c_str()) {}
  explicit RenderContextException(const char* msg) noexcept : std::runtime_error(msg) {}
};

struct ContextOpenGLDescription {
  int MajorVersion = 4;
  int MinorVersion = 6;
};

struct DepthState {
  bool IsEnableDepthTest = true;
  bool IsEnableDepthWrite = true;
  DepthComparison Comparison = DepthComparison::Less;
};

struct PipelineState {
  DepthState Depth;
  PrimitiveMode Primitive = PrimitiveMode::Triangles;
};

struct VertexBufferLayout {
  int Stride = 0;
  int Offset = 0;
  AttributeSemantic Semantic;
  VertexBufferLayout() noexcept = default;
  constexpr VertexBufferLayout(AttributeSemantic semantic, int stride, int offset) noexcept {
    Semantic = semantic;
    Stride = stride;
    Offset = offset;
  }
};

//TODO::Unifrom Buffer Object
class RenderContextOpenGL {
 public:
  RenderContextOpenGL() noexcept;
  RenderContextOpenGL(const RenderContextOpenGL&) = delete;
  RenderContextOpenGL(RenderContextOpenGL&&) noexcept;
  RenderContextOpenGL& operator=(RenderContextOpenGL&&) noexcept;
  ~RenderContextOpenGL() noexcept;

  bool IsValid() const;
  void Destroy();

  std::shared_ptr<BufferOpenGL> CreateEmptyBuffer(BufferType type,
                                                  BufferUsage usage = BufferUsage::Static,
                                                  BufferAccess access = BufferAccess::Immutable);
  std::shared_ptr<BufferOpenGL> CreateBuffer(const void* data, size_t size, BufferType type,
                                             BufferUsage usage = BufferUsage::Static,
                                             BufferAccess access = BufferAccess::Immutable);
  std::shared_ptr<BufferOpenGL> CreateVertexBuffer(const void* data, size_t size,
                                                   BufferUsage usage = BufferUsage::Static,
                                                   BufferAccess access = BufferAccess::Immutable);
  std::shared_ptr<BufferOpenGL> CreateIndexBuffer(const void* data, size_t size,
                                                  BufferUsage usage = BufferUsage::Static,
                                                  BufferAccess access = BufferAccess::Immutable);
  std::shared_ptr<ProgramOpenGL> CreateShaderProgram(const std::string& vs, const std::string& fs,
                                                     const ShaderAttributeLayouts& desc);
  std::shared_ptr<TextureOpenGL> CreateTexture2D(const Texture2dDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> CreateDepthTexture(const DepthTextureDescriptorOpenGL& desc);
  std::shared_ptr<FrameBufferOpenGL> CreateFrameBuffer(const FrameBufferDepthDescriptor& desc);
  void DestroyObject(const std::shared_ptr<ObjectOpenGL>& ptr);
  void DestroyObject(const std::shared_ptr<ProgramOpenGL>& ptr);
  std::optional<const VertexArrayOpenGL*> TryGetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;
  const VertexArrayOpenGL& GetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;

  void SetClearColor(float r, float g, float b, float a) const;
  void ClearColor() const;
  void ClearDepth(float depth) const;
  void ClearDepth() const;
  void ClearColorAndDepth() const;
  void SetViewport(int x, int y, int width, int height) const;
  void ColorMask(bool r, bool g, bool b, bool a) const;

  void DrawArrays(PrimitiveMode, int first, int count) const;
  void DrawElements(PrimitiveMode, int count, IndexDataType = IndexDataType::UnsignedInt, size_t first = 0) const;

 private:
  void CheckInit() const;
  void AddObjectToSet(const std::shared_ptr<ObjectOpenGL>& obj);

  std::unordered_set<std::shared_ptr<ObjectOpenGL>> _objects;
  std::unordered_map<std::shared_ptr<ProgramOpenGL>, VertexArrayOpenGL> _vaos;
  bool _isValid{};
  friend class Application;
};

//在buffer中排列：PNTPNTPNT
struct VertexPositionNormalTexCoord {
  Vector3f Position;
  Vector3f Normal;
  Vector2f TexCoord;
};
constexpr int SizePNT() { return static_cast<int>(sizeof(VertexPositionNormalTexCoord)); }
void CreateVboAndIboFromModelPNT(const ImmutableModel& model, BufferOpenGL& vbo, BufferOpenGL& ibo);
void CreateVboFormModelPNT(const ImmutableModel& model, BufferOpenGL& vbo);
constexpr VertexBufferLayout GetVertexLayoutPositionPNT() {
  return VertexBufferLayout({SemanticType::Vertex, 0}, SizePNT(), offsetof(VertexPositionNormalTexCoord, Position));
}
constexpr VertexBufferLayout GetVertexLayoutNormalPNT() {
  return VertexBufferLayout({SemanticType::Normal, 0}, SizePNT(), offsetof(VertexPositionNormalTexCoord, Normal));
}
constexpr VertexBufferLayout GetVertexLayoutTexCoordPNT(int index) {
  return VertexBufferLayout({SemanticType::TexCoord, index}, SizePNT(), offsetof(VertexPositionNormalTexCoord, TexCoord));
}

}  // namespace Hikari