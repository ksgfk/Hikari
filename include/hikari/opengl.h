#pragma once

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <memory>
#include <stdexcept>

#include <hikari/opengl_header.h>

//HIKARI_CHECK_GL宏，用于检查GL函数调用异常
#if defined(HIKARI_CHECK_GL)
#error "multiple define HIKARI_CHECK_GL!"
#else
#if defined(NDEBUG)
#define HIKARI_CHECK_GL(func) func
#else
#define HIKARI_CHECK_GL(func) \
  func;                       \
  (void)CheckGLError(#func, __FILE__, __LINE__)
#endif  // defined(NDEBUG)
#endif  // defined(CHECK_GL)

namespace Hikari {
class ObjectOpenGL;
class BufferOpenGL;
class ShaderOpenGL;
class ProgramOpenGL;
class VertexArrayOpenGL;
class TextureOpenGL;
class FrameBufferOpenGL;

class OpenGLException : public std::runtime_error {
 public:
  explicit OpenGLException(const std::string& msg) noexcept : std::runtime_error(msg.c_str()) {}
  explicit OpenGLException(const char* msg) noexcept : std::runtime_error(msg) {}
};

class FeatureOpenGL {
 public:
  FeatureOpenGL(const FeatureOpenGL&) = delete;
  FeatureOpenGL(FeatureOpenGL&&) = delete;
  ~FeatureOpenGL() noexcept;

  void Init();

  int GetMajorVersion() const;
  int GetMinorVersion() const;
  int GetMinUboOffsetAlign() const;
  int GetSsboAlign() const;
  const std::string& GetHardwareInfo() const;
  const std::string& GetDriverInfo() const;
  bool IsExtensionSupported(const std::string& extName) const;
  bool CanUseSsbo() const;
  bool CanEnableDebug() const;
  bool CanUseDirectStateAccess() const;
  bool CanUseBufferStorage() const;
  bool CanUseVertexAttribBinding() const;
  bool CanUseTextureStorage() const;

  static FeatureOpenGL& Get() noexcept;

 private:
  FeatureOpenGL() noexcept;
  bool _isInit{};
  int _major{};
  int _minor{};
  int _minUboOffsetAlign{};
  int _ssboAlign{};
  std::string _driverInfo;
  std::string _deviceInfo;
  std::unordered_set<std::string> _extensions;
};

class ObjectOpenGL : public std::enable_shared_from_this<ObjectOpenGL> {
 public:
  ObjectOpenGL() noexcept;
  ObjectOpenGL(const ObjectOpenGL&) = delete;
  virtual ~ObjectOpenGL() noexcept = 0;
  virtual bool IsValid() const = 0;
  virtual void Destroy() = 0;
};

void CheckGLError(const char* callFuncName, const char* fileName, int line);

enum class PrimitiveMode {
  Triangles
};

enum class IndexDataType {
  UnsignedByte,
  UnsignedShort,
  UnsignedInt
};

enum class DepthComparison {
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always,
};

GLenum MapPrimitiveMode(PrimitiveMode);
GLenum MapIndexDataType(IndexDataType);
GLenum MapComparison(DepthComparison);

enum class BufferType {
  Unknown,
  VertexBuffer,
  IndexBuffer,
  UniformBuffer
};

enum class BufferUsage {
  Static,
  Dynamic
};

enum class BufferAccess {
  Immutable,
  CpuReadOnly,
  CpuWriteOnly,
  CpuReadWrite
};

class BufferOpenGL : public ObjectOpenGL {
 public:
  BufferOpenGL() noexcept;
  BufferOpenGL(BufferType, BufferUsage = BufferUsage::Static, BufferAccess = BufferAccess::Immutable);
  BufferOpenGL(const void*, size_t, BufferType, BufferUsage = BufferUsage::Static, BufferAccess = BufferAccess::Immutable);
  BufferOpenGL(BufferOpenGL&&) noexcept;
  BufferOpenGL& operator=(BufferOpenGL&&) noexcept;
  ~BufferOpenGL() noexcept override;
  bool IsValid() const override;
  void Destroy() override;

  GLuint GetHandle() const noexcept;
  BufferType GetType() const noexcept;
  BufferUsage GetUsage() const noexcept;
  BufferAccess GetAccess() const noexcept;
  void Bind() const noexcept;

  void Store(const void*, size_t);

  static GLenum MapTypeToTarget(BufferType);
  static GLenum MapToUsage(BufferUsage, BufferAccess);
  static GLbitfield MapToBitField(BufferUsage, BufferAccess);

 private:
  void Delete();
  GLuint _handle{};
  BufferType _type{};
  BufferUsage _usage{};
  BufferAccess _access{};
};

enum class ShaderType {
  Unknown,
  Fragment,
  Vertex
};

class ShaderOpenGL : public ObjectOpenGL {
 public:
  ShaderOpenGL() noexcept;
  ShaderOpenGL(ShaderType type, const std::string& source);
  ShaderOpenGL(ShaderOpenGL&&) noexcept;
  ShaderOpenGL& operator=(ShaderOpenGL&&) noexcept;
  ~ShaderOpenGL() noexcept override;
  bool IsValid() const override;
  void Destroy() override;

  GLuint GetHandle() const;
  ShaderType GetType() const;

  static GLenum MapType(ShaderType type);
  static bool CompileFromSource(GLenum type, const std::string& source, GLuint& result);

 private:
  void Delete();
  ShaderType _type = ShaderType::Unknown;
  GLuint _handle{};
};

enum class ParamType {
  Unknown,
  Int32,
  Int32Vec2,
  Int32Vec3,
  Int32Vec4,
  Float32,
  Float32Vec2,
  Float32Vec3,
  Float32Vec4,
  Float32Mat2,
  Float32Mat3,
  Float32Mat4,
  Sampler2d,
  SamplerCubeMap
};

enum class SemanticType {
  Unknown,
  Vertex,
  Normal,
  TexCoord,
  Color
};

struct AttributeSemantic {
  SemanticType Type = SemanticType::Unknown;
  int Index{};
  AttributeSemantic() noexcept = default;
  constexpr AttributeSemantic(SemanticType type, int index) noexcept {
    Type = type;
    Index = index;
  }
  bool operator==(const AttributeSemantic& o) const {
    return Type == o.Type && Index == o.Index;
  }
  bool operator!=(const AttributeSemantic& o) const {
    return Type != o.Type || Index != o.Index;
  }
};

struct SemanticHash {
  constexpr size_t operator()(const AttributeSemantic& semantic) const {
    auto type = (int)semantic.Type;
    auto index = semantic.Index;
    return index ^ (type << 8);
  }
};

struct ShaderAttributeLayout {
  std::string Name;
  AttributeSemantic Semantic{};
  ShaderAttributeLayout() noexcept;
  ShaderAttributeLayout(const std::string& name, SemanticType type, int index) noexcept;
};

using ShaderAttributeLayouts = std::vector<ShaderAttributeLayout>;

struct ShaderAttribute {
  std::string Name;
  ParamType Type = ParamType::Unknown;
  int Length = 0;
  int Location = -1;
  AttributeSemantic Semantic{};
};

struct ShaderUniform {
  std::string Name;
  ParamType Type = ParamType::Unknown;
  int Length = 0;
  int Location = -1;
};

class ProgramOpenGL : public ObjectOpenGL {
 public:
  using AttribInfo = ShaderUniform;
  using UniformInfo = ShaderUniform;
  using AttribInfoArray = std::vector<AttribInfo>;    //只用来记录attrib反射的信息
  using UniformInfoArray = std::vector<UniformInfo>;  //只用来记录uniform反射的信息

  ProgramOpenGL() noexcept;
  ProgramOpenGL(const ShaderOpenGL& vs, const ShaderOpenGL& fs, const ShaderAttributeLayouts& desc);
  ProgramOpenGL(ProgramOpenGL&&) noexcept;
  ProgramOpenGL& operator=(ProgramOpenGL&&) noexcept;
  ~ProgramOpenGL() noexcept override;
  bool IsValid() const override;
  void Destroy() override;

  GLuint GetHandle() const;
  void Bind() const;
  std::optional<const ShaderAttribute*> GetAttribute(const std::string&) const;
  std::optional<const ShaderAttribute*> GetAttribute(AttributeSemantic) const;
  int GetBindingPoint(AttributeSemantic) const;
  GLuint GetAttributeLocation(const std::string&) const;
  GLuint GetAttributeLocation(AttributeSemantic) const;
  std::optional<const ShaderUniform*> GetUniform(const std::string&) const;
  constexpr const std::vector<ShaderAttribute>& GetAttributes() const { return _attribs; }
  constexpr const std::vector<ShaderUniform>& GetUniforms() const { return _uniforms; }

  static void Uniform(GLuint prog, GLint location, ParamType type, GLsizei count, const void* value);
  static void UniformMat4(GLuint prog, GLint index, const float* value);
  static void Uniform(GLuint prog, GLint index, float value);
  static void Uniform(GLuint prog, GLint index, int value);
  static void UniformVec3(GLuint prog, GLint index, const float* value);
  void UniformMat4(const std::string& name, const float* value) const;
  void Uniform(const std::string& name, float value) const;
  void Uniform(const std::string& name, int value) const;
  void UniformVec3(const std::string& name, const float* value) const;
  void UniformTexture2D(const std::string& name, GLuint handle) const;
  void UniformCubeMap(const std::string& name, GLuint handle) const;

  static ParamType MapType(GLenum type);
  static bool Link(const ShaderOpenGL& vs,
                   const ShaderOpenGL& fs,
                   const ShaderAttributeLayouts& desc,
                   ProgramOpenGL& result);
  static AttribInfoArray ReflectActiveAttrib(GLuint prog);
  static UniformInfoArray ReflectActiveUniform(GLuint prog);

 private:
  void Delete();
  GLuint _handle{};
  std::vector<ShaderAttribute> _attribs;
  std::vector<ShaderUniform> _uniforms;
  std::unordered_map<std::string_view, size_t> _nameToAttrib;
  std::unordered_map<AttributeSemantic, size_t, SemanticHash> _semanticToAttrib;
  std::unordered_map<std::string_view, size_t> _nameToUni;
};

struct VertexBufferBinding {
  GLuint BindingPoint = 0;
  GLuint Handle = 0;  //VBO的Handle
  BufferType Type = BufferType::Unknown;
  GLintptr Offset = 0;  //以字节为单位
  GLsizei Stride = 0;
  VertexBufferBinding() noexcept;
  //VBO
  VertexBufferBinding(GLuint point, GLuint handle, GLintptr offset = 0, GLsizei stride = 0) noexcept;
  //IBO
  VertexBufferBinding(GLuint handle, GLintptr offset = 0, GLsizei stride = 0) noexcept;
};

struct VertexAttributeFormat {
  GLuint AttribIndex;
  ParamType Type;
  GLboolean IsNormalised;
  GLuint RelativeOffset;  //以字节为单位
};

struct VertexAssociate {
  GLuint AttribIndex;
  GLuint BindingPoint;
};

using VertexBufferBindings = std::vector<VertexBufferBinding>;
using VertexAttributeFormats = std::vector<VertexAttributeFormat>;
using VertexAssociates = std::vector<VertexAssociate>;

class VertexArrayOpenGL : public ObjectOpenGL {
 public:
  VertexArrayOpenGL() noexcept;
  //所有数据一次性绑死，不建议使用
  VertexArrayOpenGL(const VertexBufferBindings&, const VertexAttributeFormats&, const VertexAssociates&);
  //假设binding point和attribute location是相等的
  VertexArrayOpenGL(const std::vector<ShaderAttribute>& attribs);
  VertexArrayOpenGL(VertexArrayOpenGL&&) noexcept;
  VertexArrayOpenGL& operator=(VertexArrayOpenGL&&) noexcept;
  ~VertexArrayOpenGL() noexcept override;
  bool IsValid() const override;
  void Destroy() override;

  GLuint GetHandle() const;
  void Bind() const;
  //如果opengl版本小于4.5，应该确保调用该函数前，绑定的VAO是正确的
  void SetVertexBuffer(const VertexBufferBinding& binding) const;
  //如果opengl版本小于4.5，应该确保调用该函数前，绑定的VAO是正确的
  void SetIndexBuffer(GLuint iboHandle) const;

  static std::pair<GLint, GLenum> MapType(ParamType type);

 private:
  void Delete();
  GLuint _handle{};
  std::unordered_map<GLuint, VertexAttributeFormat> _attribFormat;
};

enum class TextureType {
  Unknown,
  Image2d,
  CubeMap
};

enum class WrapMode {
  Repeat,
  Clamp,
  Mirror
};

enum class FilterMode {
  Point,
  Bilinear,
  Trilinear  //各向异性呢
};

enum class ColorFormat {
  RGB,
  RGBA,
  Depth
};

enum class PixelFormat : GLenum {
  RGB8 = GL_RGB8,
  RGBA8 = GL_RGBA8,
  Depth16 = GL_DEPTH_COMPONENT16,
  Depth24 = GL_DEPTH_COMPONENT24,
  Depth32 = GL_DEPTH_COMPONENT32,
};

enum class ImageDataType {
  Byte,
  Float32
};

struct Texture2dDescriptorOpenGL {
  WrapMode Wrap;
  FilterMode MinFilter;
  FilterMode MagFilter;
  int MipMapLevel;
  PixelFormat TextureFormat;
  int Width;
  int Height;
  ColorFormat DataFormat;
  ImageDataType DataType;
  void* DataPtr;
};

struct TextureCubeMapDescriptorOpenGL {
  WrapMode Wrap;
  FilterMode MinFilter;
  FilterMode MagFilter;
  int MipMapLevel;
  PixelFormat TextureFormat;
  //为了方便，cubemap所有面纹理长宽都应该一致
  int Width;
  int Height;
  ColorFormat DataFormat[6];
  ImageDataType DataType[6];
  void* DataPtr[6];
};

struct DepthTextureDescriptorOpenGL {
  WrapMode Wrap;
  FilterMode MinFilter;
  FilterMode MagFilter;
  int Width;
  int Height;
  PixelFormat TextureFormat;
  ImageDataType DataType;
};

class TextureOpenGL : public ObjectOpenGL {
 public:
  TextureOpenGL() noexcept;
  TextureOpenGL(const Texture2dDescriptorOpenGL& desc);
  TextureOpenGL(const TextureCubeMapDescriptorOpenGL& desc);
  TextureOpenGL(const DepthTextureDescriptorOpenGL& desc);
  TextureOpenGL(TextureOpenGL&&) noexcept;
  TextureOpenGL& operator=(TextureOpenGL&&) noexcept;
  ~TextureOpenGL() noexcept override;
  void Destroy() override;
  bool IsValid() const override;

  GLuint GetHandle() const;
  TextureType GetType() const;
  int GetWidth() const;
  int GetHeight() const;

  static GLuint MapFilterMode(FilterMode mode);
  static GLuint MapWrapMode(WrapMode mode);
  static GLint MapPixelFormat(ColorFormat format);
  static GLenum MapTextureDataType(ImageDataType format);
  static GLsizei CalcMipmapLevels(int mipmapLevel, int maxSize, bool isUseTrilinear);
  static void CreateTexture2d(const Texture2dDescriptorOpenGL& desc, TextureOpenGL& texture);
  static void CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc, TextureOpenGL& texture);
  static GLenum MapTextureType(TextureType type);

 private:
  void Delete();
  GLuint _handle{};
  TextureType _type{};
  int _width{};
  int _height{};
};

struct FrameBufferDepthDescriptor {
  GLuint Texture2D;
};

class FrameBufferOpenGL : public ObjectOpenGL {
 public:
  FrameBufferOpenGL() noexcept;
  FrameBufferOpenGL(const FrameBufferDepthDescriptor& depth);
  FrameBufferOpenGL(FrameBufferOpenGL&&) noexcept;
  FrameBufferOpenGL& operator=(FrameBufferOpenGL&&) noexcept;
  ~FrameBufferOpenGL() noexcept override;
  bool IsValid() const override;
  void Destroy() override;

  void Bind() const;
  void Unbind() const;

 private:
  void Delete();
  GLuint _handle{};
};

}  // namespace Hikari