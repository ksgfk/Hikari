#pragma once

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <optional>
#include <filesystem>

#include <glslang/Public/ShaderLang.h>

#include <hikari/mathematics.h>
#include <hikari/opengl.h>

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

struct GlobalUniformBlock {
  ShaderUniformBlock Block{};
  size_t BindingPoint = std::numeric_limits<size_t>::max();
  std::shared_ptr<BufferOpenGL> Ubo;
  std::vector<uint8_t> Data;  //用来debug（
};

struct GlobalUniform {
  ShaderUniformBlock::Member Info;
  size_t BlockHandle;
};

class ShaderIncluder : public glslang::TShader::Includer {
 public:
  ShaderIncluder() noexcept;
  ~ShaderIncluder() override;

  IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override;
  IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override;
  void releaseInclude(IncludeResult*) override;

  void AddSystemPath(const std::filesystem::path& sysPath);

  static std::string ReadText(const std::filesystem::path& p);

 private:
  std::vector<std::filesystem::path> _systemPaths;
  std::filesystem::path _workPath;
};

class RenderContextOpenGL {
 public:
  RenderContextOpenGL() noexcept;
  RenderContextOpenGL(const RenderContextOpenGL&) = delete;
  RenderContextOpenGL(RenderContextOpenGL&&) noexcept;
  RenderContextOpenGL& operator=(RenderContextOpenGL&&) noexcept;
  ~RenderContextOpenGL() noexcept;

  void Init(const std::filesystem::path& shaderPath);
  bool IsValid() const;
  void Destroy();

  std::shared_ptr<BufferOpenGL> CreateBuffer(const void* data, size_t size, BufferType type,
                                             BufferUsage usage = BufferUsage::Static,
                                             BufferAccess access = BufferAccess::NoMap);
  std::shared_ptr<BufferOpenGL> CreateVertexBuffer(const void* data, size_t size,
                                                   BufferUsage usage = BufferUsage::Static,
                                                   BufferAccess access = BufferAccess::NoMap);
  std::shared_ptr<BufferOpenGL> CreateIndexBuffer(const void* data, size_t size,
                                                  BufferUsage usage = BufferUsage::Static,
                                                  BufferAccess access = BufferAccess::NoMap);
  std::shared_ptr<BufferOpenGL> CreateUniformBuffer(const void* data, size_t size,
                                                    BufferUsage usage = BufferUsage::Static,
                                                    BufferAccess access = BufferAccess::NoMap);
  std::shared_ptr<ProgramOpenGL> CreateShaderProgram(const std::string& vs, const std::string& fs,
                                                     const ShaderAttributeLayouts& desc);
  std::shared_ptr<TextureOpenGL> CreateTexture2D(const Texture2dDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> CreateDepthTexture(const DepthTextureDescriptorOpenGL& desc);
  std::shared_ptr<FrameBufferOpenGL> CreateFrameBuffer(const FrameBufferDepthDescriptor& desc);
  void AddUniformBlocks(const ProgramOpenGL& prog);
  void DestroyObject(const std::shared_ptr<ObjectOpenGL>& ptr);
  void DestroyObject(const std::shared_ptr<ProgramOpenGL>& ptr);

  std::optional<const VertexArrayOpenGL*> TryGetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;
  const VertexArrayOpenGL& GetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;

  /**
   * @brief 预处理shader。应用宏替换，处理include指令
   * @param type 阶段（shader stage）
   * @param source glsl源码
   * @param res 编译后的glsl源码，如果编译失败则返回错误信息
   * @return 是否编译成功
  */
  bool ProprocessShader(ShaderType type, const std::string& source, std::string& res);
  /**
   * @brief 完整编译shader，输入的glsl必须可以编译为SPIR-V
   * @param type 阶段（shader stage）
   * @param source glsl源码
   * @param res 编译后的glsl源码，如果编译失败则返回错误信息
   * @return 是否编译成功
  */
  bool ProcessShader(ShaderType type, const std::string& source, std::string& res);

  void SetClearColor(float r, float g, float b, float a) const;
  void ClearColor() const;
  void ClearDepth(float depth) const;
  void ClearDepth() const;
  void ClearColorAndDepth() const;
  void SetViewport(int x, int y, int width, int height) const;
  void ColorMask(bool r, bool g, bool b, bool a) const;

  void DrawArrays(PrimitiveMode, int first, int count) const;
  void DrawElements(PrimitiveMode, int count, IndexDataType = IndexDataType::UnsignedInt, size_t first = 0) const;

  void SubmitGlobalUnifroms() const;
  void SetGlobalUniformData(const std::string& name, size_t dataSize, int length, const void* data);
  void SetGlobalUniformData(const GlobalUniform& uniform, size_t dataSize, int length, const void* data);
  void SetGlobalUniform(const std::string& name, size_t dataSize, int length, const void* data);
  void SetGlobalFloat(const std::string& name, float value);
  void SetGlobalInt(const std::string& name, int value);
  void SetGlobalMat4(const std::string& name, const Matrix4f& value);
  void SetGlobalVec3(const std::string& name, const Vector3f& value);
  void SetGlobalTex2d(const std::string& name, const TextureOpenGL& tex2d);
  void SetGlobalCubeMap(const std::string& name, const TextureOpenGL& cubemap);
  void SetGlobalFloatArray(const std::string& name, const float* value, int length);
  void SetGlobalIntArray(const std::string& name, const int* value, int length);
  void SetGlobalMat4Array(const std::string& name, const Matrix4f* value, int length);
  void SetGlobalVec3Array(const std::string& name, const Vector3f* value, int length);
  void SetGlobalTex2dArray(const std::string& name, const GLuint* tex2d, int length);
  void SetGlobalCubeMapArray(const std::string& name, const GLuint* cubemap, int length);

 private:
  void CheckInit() const;
  void AddObjectToSet(const std::shared_ptr<ObjectOpenGL>& obj);

  ShaderIncluder _includer;
  std::unordered_set<std::shared_ptr<ObjectOpenGL>> _objects;
  std::unordered_map<std::shared_ptr<ProgramOpenGL>, VertexArrayOpenGL> _vaos;
  std::vector<GlobalUniformBlock> _globalBlocks;
  std::unordered_map<std::string, size_t> _blockQueryMap;
  std::unordered_map<std::string, GlobalUniform> _globalUniforms;
  bool _isValid{};
};

//在buffer中排列：PNTPNTPNT
struct VertexPositionNormalTexCoord {
  Vector3f Position;
  Vector3f Normal;
  Vector2f TexCoord;
};
constexpr int SizePNT() { return static_cast<int>(sizeof(VertexPositionNormalTexCoord)); }
std::vector<VertexPositionNormalTexCoord> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                                        const std::vector<Vector3f>& normal,
                                                        const std::vector<Vector2f>& tex);
std::vector<VertexPositionNormalTexCoord> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                                        const std::vector<Vector3f>& normal,
                                                        const std::vector<Vector2f>& tex,
                                                        const std::vector<size_t>& idx);
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