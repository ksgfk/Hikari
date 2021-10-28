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
struct VertexPNT;
struct VertexPTNT;

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

struct GBufferLayout {
  std::string Stage;
  PixelFormat Format;
  ImageDataFormat DataFormat;
  ImageDataType DataType;
};

class GBuffer {
 public:
  int Width{};
  int Height{};
  std::vector<GBufferLayout> Layouts;
  std::vector<std::shared_ptr<TextureOpenGL>> Buffers;
  std::shared_ptr<RenderBufferOpenGL> Depth;
  std::shared_ptr<FrameBufferOpenGL> Frame;
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
  std::shared_ptr<ProgramOpenGL> CreateShaderProgram(const std::string& vs,
                                                     const std::string& fs,
                                                     const ShaderAttributeLayouts& desc);
  std::shared_ptr<ProgramOpenGL> LoadShaderProgram(const std::filesystem::path& vsPath,
                                                   const std::filesystem::path& fsPath,
                                                   const std::filesystem::path& libPath,
                                                   const ShaderAttributeLayouts& desc,
                                                   const std::vector<std::string>& macros = {});
  std::shared_ptr<TextureOpenGL> CreateTexture2D(const Texture2dDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> LoadBitmap2D(std::filesystem::path, WrapMode, FilterMode, PixelFormat);
  std::shared_ptr<TextureOpenGL> CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc);
  std::shared_ptr<TextureOpenGL> CreateDepthTexture(const DepthTextureDescriptorOpenGL& desc);
  std::shared_ptr<FrameBufferOpenGL> CreateFrameBuffer(const FrameBufferDepthDescriptor& desc);
  std::shared_ptr<FrameBufferOpenGL> CreateFrameBuffer(const FrameBufferRenderDescriptor& desc);
  std::shared_ptr<FrameBufferOpenGL> CreateFrameBuffer(GLuint handle);
  std::shared_ptr<RenderBufferOpenGL> CreateRenderBuffer(const RenderBufferDescriptor& desc);
  std::unique_ptr<GBuffer> CreateGBuffer(Vector2i size, const std::vector<GBufferLayout>& layouts, bool hasDepth);
  std::shared_ptr<BufferOpenGL> CreateCubeVbo(float halfExtend, int& vertexCnt);
  std::shared_ptr<BufferOpenGL> CreateQuadVbo(float halfExtend, int& vertexCnt);
  std::shared_ptr<BufferOpenGL> CreateVbo(const std::vector<VertexPNT>& pnt);
  std::shared_ptr<BufferOpenGL> CreateVbo(const std::vector<VertexPTNT>& ptnt);
  /**
   * @brief 将等距柱状投影图转换为立方体图
  */
  std::shared_ptr<TextureOpenGL> ConvertSphericalToCubemap(
      const Texture2dDescriptorOpenGL& tex2d,
      const TextureCubeMapDescriptorOpenGL& cubeConfig,
      const std::filesystem::path& shaderLib);
  /**
   * @brief 生成辐照度卷积
  */
  std::shared_ptr<TextureOpenGL> GenIrradianceConvolutionCubemap(
      const std::shared_ptr<TextureOpenGL>& irradiance,
      const TextureCubeMapDescriptorOpenGL& cubeConfig,
      const std::filesystem::path& shaderLib);
  /**
   * @brief 预滤波环境贴图
  */
  std::shared_ptr<TextureOpenGL> PrefilterEnvMap(
      const std::shared_ptr<TextureOpenGL>& env,
      const TextureCubeMapDescriptorOpenGL& config,
      const std::filesystem::path& shaderLib);
  /**
   * @brief 预计算BRDF查询表
  */
  std::shared_ptr<TextureOpenGL> PrecomputeBrdfLut(const Texture2dDescriptorOpenGL& desc,
                                                   const std::filesystem::path& shaderLib);
  /**
   * @brief 预计算多次散射BRDF查询表
  */
  std::shared_ptr<TextureOpenGL> PrecomputerBrdfMultiScatteringLut(
      const Texture2dDescriptorOpenGL& desc,
      const std::filesystem::path& shaderLib);
  void AddUniformBlocks(const ProgramOpenGL& prog);
  void DestroyObject(const std::shared_ptr<ObjectOpenGL>& ptr);
  void DestroyObject(const std::shared_ptr<ProgramOpenGL>& ptr);

  std::optional<const VertexArrayOpenGL*> TryGetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;
  const VertexArrayOpenGL& GetVertexArray(const std::shared_ptr<ProgramOpenGL>&) const;

  /**
   * @brief 预处理shader。应用宏替换，处理include指令，初步验证语法正确性
   * @param type 阶段（shader stage）
   * @param source glsl源码
   * @param res 预处理后glsl源码，如果处理失败则不返回任何数据
   * @param args 宏
   * @return 是否预处理成功
  */
  bool PreprocessShader(
      ShaderType type,
      const std::string& source,
      std::string& res,
      const std::vector<std::string>& args = {});
  /**
   * @brief 完整编译shader，输入的glsl必须可以编译为SPIR-V
   * @param type 阶段（shader stage）
   * @param source glsl源码
   * @param res 编译后的glsl源码，如果处理失败则不返回任何数据
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
  void SetGlobalUniformData(const std::string& name, size_t dataSize, int length, int align, const void* data);
  /**
   * @brief 将uniform block需要的数据传入buffer
   * @param uniform uniform对象
   * @param dataSize data指向数据类型的大小
   * @param length data指向数据长度
   * @param align 如果数据是数组，表示两元素间的距离
   * @param data 数据指针
   * @return 
  */
  void SetGlobalUniformData(const GlobalUniform& uniform, size_t dataSize, int length, int align, const void* data);
  void SetGlobalUniform(const std::string& name, size_t dataSize, int length, int align, const void* data);
  void SetGlobalFloat(const std::string& name, float value);
  void SetGlobalInt(const std::string& name, int value);
  void SetGlobalMat4(const std::string& name, const Matrix4f& value);
  void SetGlobalVec3(const std::string& name, const Vector3f& value);
  void SetGlobalTex2d(const std::string& name, const TextureOpenGL& tex2d);
  void SetGlobalCubeMap(const std::string& name, const TextureOpenGL& cubemap);
  void SetGlobalFloatArray(const std::string& name, const void* value, int length);
  void SetGlobalIntArray(const std::string& name, const void* value, int length);
  void SetGlobalMat4Array(const std::string& name, const void* value, int length);
  void SetGlobalVec3Array(const std::string& name, const void* value, int length);
  void SetGlobalTex2dArray(const std::string& name, const void* tex2d, int length);
  void SetGlobalCubeMapArray(const std::string& name, const void* cubemap, int length);

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

//一些shader library包含的uniform名
constexpr const char* UNIFORM_MODEL_MATRIX = "u_ObjectToWorld";
constexpr const char* UNIFORM_MODEL_MATRIX_INV = "u_WorldToObject";
constexpr const char* UNIFORM_VIEW_MATRIX = "u_MatrixV";
constexpr const char* UNIFORM_VIEW_MATRIX_INV = "u_MatrixInvV";
constexpr const char* UNIFORM_PROJ_MATRIX = "u_MatrixP";
constexpr const char* UNIFORM_VP_MATRIX = "u_MatrixVP";
constexpr const char* UNIFORM_CAMERA_POS = "u_CameraPos";
constexpr const char* UNIFORM_LIGHT_DIR_RAD = "u_LightRadianceDir";
constexpr const char* UNIFORM_LIGHT_DIR_DIR = "u_LightDirectionDir";
constexpr const char* UNIFORM_LIGHT_DIR_CNT = "u_LightDirCount";
constexpr const char* UNIFORM_LIGHT_POINT_RAD = "u_LightRadiancePoint";
constexpr const char* UNIFORM_LIGHT_POINT_DIR = "u_LightPositionPoint";
constexpr const char* UNIFORM_LIGHT_POINT_CNT = "u_LightPointCount";

//在buffer中排列：PNTPNTPNT
struct VertexPNT {
  Vector3f Position;
  Vector3f Normal;
  Vector2f TexCoord;
};
constexpr int SizePNT() { return static_cast<int>(sizeof(VertexPNT)); }
std::vector<VertexPNT> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                     const std::vector<Vector3f>& normal,
                                     const std::vector<Vector2f>& tex);
std::vector<VertexPNT> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                     const std::vector<Vector3f>& normal,
                                     const std::vector<Vector2f>& tex,
                                     const std::vector<size_t>& idx);
constexpr VertexBufferLayout GetVertexPosPNT() {
  return VertexBufferLayout({SemanticType::Vertex, 0}, SizePNT(), offsetof(VertexPNT, Position));
}
constexpr VertexBufferLayout GetVertexNormalPNT() {
  return VertexBufferLayout({SemanticType::Normal, 0}, SizePNT(), offsetof(VertexPNT, Normal));
}
constexpr VertexBufferLayout GetVertexTexPNT(int index) {
  return VertexBufferLayout({SemanticType::TexCoord, index}, SizePNT(), offsetof(VertexPNT, TexCoord));
}
//在buffer中排列：PTNT PTNT PTNT
struct VertexPTNT {
  Vector3f Position;
  Vector4f Tangent;
  Vector3f Normal;
  Vector2f TexCoord;
};
constexpr int SizePTNT() { return static_cast<int>(sizeof(VertexPTNT)); }
std::vector<VertexPTNT> GenVboDataPTNT(const std::vector<Vector3f>& pos,
                                       const std::vector<Vector4f>& tan,
                                       const std::vector<Vector3f>& normal,
                                       const std::vector<Vector2f>& tex);
std::vector<VertexPTNT> GenVboDataPTNT(const std::vector<Vector3f>& pos,
                                       const std::vector<Vector4f>& tan,
                                       const std::vector<Vector3f>& normal,
                                       const std::vector<Vector2f>& tex,
                                       const std::vector<size_t>& idx);
constexpr VertexBufferLayout GetVertexPosPTNT() {
  return VertexBufferLayout({SemanticType::Vertex, 0}, SizePTNT(), offsetof(VertexPTNT, Position));
}
constexpr VertexBufferLayout GetVertexTanPTNT() {
  return VertexBufferLayout({SemanticType::Tangent, 0}, SizePTNT(), offsetof(VertexPTNT, Tangent));
}
constexpr VertexBufferLayout GetVertexNormalPTNT() {
  return VertexBufferLayout({SemanticType::Normal, 0}, SizePTNT(), offsetof(VertexPTNT, Normal));
}
constexpr VertexBufferLayout GetVertexTexPTNT(int index) {
  return VertexBufferLayout({SemanticType::TexCoord, index}, SizePTNT(), offsetof(VertexPTNT, TexCoord));
}

ShaderAttributeLayout POSITION();
ShaderAttributeLayout TANGENT();
ShaderAttributeLayout NORMAL();
ShaderAttributeLayout TEXCOORD0();

}  // namespace Hikari