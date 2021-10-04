#include <hikari/opengl.h>

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

namespace Hikari {
FeatureOpenGL::FeatureOpenGL() noexcept = default;

FeatureOpenGL::~FeatureOpenGL() noexcept = default;

void FeatureOpenGL::Init() {
  _extensions.clear();
  HIKARI_CHECK_GL(glGetIntegerv(GL_MAJOR_VERSION, &_major));
  HIKARI_CHECK_GL(glGetIntegerv(GL_MINOR_VERSION, &_minor));
  auto driverInfo = HIKARI_CHECK_GL(glGetString(GL_VERSION));
  _driverInfo = std::string((char*)driverInfo);  //直接强转byte，快进到编码问题（
  auto deviceName = HIKARI_CHECK_GL(glGetString(GL_RENDERER));
  _deviceInfo = std::string((char*)deviceName);

  int extensionCount;
  HIKARI_CHECK_GL(glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount));
  for (int i = 0; i < extensionCount; i++) {
    auto extPtr = HIKARI_CHECK_GL(glGetStringi(GL_EXTENSIONS, i));
    _extensions.emplace(std::string((char*)extPtr));
  }

  HIKARI_CHECK_GL(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_minUboOffsetAlign));
  HIKARI_CHECK_GL(glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &_maxUniformLocation));
  HIKARI_CHECK_GL(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &_maxUniformBlockBindings));
  HIKARI_CHECK_GL(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &_maxUniformBlockSize));

  if (CanUseSsbo()) {
    HIKARI_CHECK_GL(glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &_ssboAlign));
  }
  _isInit = true;
}

int FeatureOpenGL::GetMajorVersion() const { return _major; }

int FeatureOpenGL::GetMinorVersion() const { return _minor; }

int FeatureOpenGL::GetMinUboOffsetAlign() const { return _minUboOffsetAlign; }

int FeatureOpenGL::GetSsboAlign() const { return _ssboAlign; }

int FeatureOpenGL::GetMaxUniformLocation() const { return _maxUniformLocation; }

int FeatureOpenGL::GetMaxUniformBlockBindings() const { return _maxUniformBlockBindings; }

int FeatureOpenGL::GetMaxUniformBlockSize() const { return _maxUniformBlockSize; }

int FeatureOpenGL::GetMaxGlslVersion() const {
  auto major = GetMajorVersion();
  auto minor = GetMinorVersion();
  int result{};
  if (major <= 3 && minor <= 3) {
    result = 330;
  } else if (major >= 4 && minor > 5) {
    result = 450;
  } else {
    result = major * 100 + minor * 10;
  }
  return result;
}

const std::string& FeatureOpenGL::GetHardwareInfo() const { return _deviceInfo; }

const std::string& FeatureOpenGL::GetDriverInfo() const { return _driverInfo; }

bool FeatureOpenGL::IsExtensionSupported(const std::string& extName) const { return _extensions.find(extName) != _extensions.end(); }

bool FeatureOpenGL::CanUseSsbo() const {
  return (_major >= 4 && _minor >= 3);  //|| IsExtensionSupported("GL_ARB_shader_storage_buffer_object");
}

bool FeatureOpenGL::CanEnableDebug() const {
  return (_major >= 4 && _minor >= 3);  //|| IsExtensionSupported("GL_ARB_debug_output");
}

bool FeatureOpenGL::CanUseDirectStateAccess() const {
  return (_major >= 4 && _minor >= 5);  //|| IsExtensionSupported("GL_ARB_direct_state_access");
}

bool FeatureOpenGL::CanUseBufferStorage() const {
  return (_major >= 4 && _minor >= 4);  //|| IsExtensionSupported("GL_ARB_buffer_storage");
}

bool FeatureOpenGL::CanUseVertexAttribBinding() const {
  return (_major >= 4 && _minor >= 3);  // || IsExtensionSupported("GL_ARB_vertex_attrib_binding");
}

bool FeatureOpenGL::CanUseTextureStorage() const {
  return (_major >= 4 && _minor >= 2);  //||IsExtensionSupported("GL_ARB_texture_storage");
}

FeatureOpenGL& FeatureOpenGL::Get() noexcept {
  static FeatureOpenGL _feature;
  return _feature;
}

GLenum MapPrimitiveMode(PrimitiveMode mode) {
  switch (mode) {
    case PrimitiveMode::Triangles:
      return GL_TRIANGLES;
    default:
      throw OpenGLException("unknown PrimitiveMode");
  }
}

GLenum MapIndexDataType(IndexDataType type) {
  switch (type) {
    case IndexDataType::UnsignedByte:
      return GL_UNSIGNED_BYTE;
    case IndexDataType::UnsignedShort:
      return GL_UNSIGNED_SHORT;
    case IndexDataType::UnsignedInt:
      return GL_UNSIGNED_INT;
    default:
      throw OpenGLException("unknown IndexDataType");
  }
}

GLenum MapComparison(DepthComparison comp) {
  switch (comp) {
    case Hikari::DepthComparison::Never:
      return GL_NEVER;
    case Hikari::DepthComparison::Less:
      return GL_LESS;
    case Hikari::DepthComparison::Equal:
      return GL_EQUAL;
    case Hikari::DepthComparison::LessEqual:
      return GL_LEQUAL;
    case Hikari::DepthComparison::Greater:
      return GL_GREATER;
    case Hikari::DepthComparison::NotEqual:
      return GL_NOTEQUAL;
    case Hikari::DepthComparison::GreaterEqual:
      return GL_GEQUAL;
    case Hikari::DepthComparison::Always:
      return GL_ALWAYS;
    default:
      throw OpenGLException("unknown DepthComparison");
  }
}

ObjectOpenGL::ObjectOpenGL() noexcept = default;

ObjectOpenGL::~ObjectOpenGL() noexcept = default;

void CheckGLError(const char* callFuncName, const char* fileName, int line) {
  GLenum err = glGetError();
  const char* msg = nullptr;
  switch (err) {
    case GL_NO_ERROR:
      return;
    case GL_INVALID_ENUM:
      msg = "invalid_enum";
      break;
    case GL_INVALID_VALUE:
      msg = "invalid_value";
      break;
    case GL_INVALID_OPERATION:
      msg = "invalid_operation";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      msg = "invalid_framebuffer_operation";
      break;
    case GL_OUT_OF_MEMORY:
      msg = "out_of_memory";
      break;
    case GL_STACK_UNDERFLOW:
      msg = "stack_underflow";
      break;
    case GL_STACK_OVERFLOW:
      msg = "stack_overflow";
      break;
    default:
      msg = "unknown error";
      break;
  }
  std::cerr << "GL error " << msg << " at " << fileName << "[" << line << "]" << std::endl;
}

BufferOpenGL::BufferOpenGL() noexcept = default;

BufferOpenGL::BufferOpenGL(const void* data, size_t size, BufferType type, BufferUsage usage, BufferAccess access) {
  _type = type;
  _usage = usage;
  _access = access;
  Store(data, size);
}

BufferOpenGL::BufferOpenGL(BufferOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
  _usage = other._usage;
  _access = other._access;
}

BufferOpenGL& BufferOpenGL::operator=(BufferOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
  _usage = other._usage;
  _access = other._access;
  return *this;
}

BufferOpenGL::~BufferOpenGL() noexcept {
  Delete();
}

bool BufferOpenGL::IsValid() const {
  return _handle != 0;
}

void BufferOpenGL::Destroy() {
  Delete();
}

void BufferOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteBuffers(1, &_handle));
    _handle = 0;
  }
}

GLuint BufferOpenGL::GetHandle() const noexcept {
  return _handle;
}

BufferType BufferOpenGL::GetType() const noexcept {
  return _type;
}

BufferUsage BufferOpenGL::GetUsage() const noexcept {
  return _usage;
}

BufferAccess BufferOpenGL::GetAccess() const noexcept {
  return _access;
}

void BufferOpenGL::Bind() const noexcept {
  HIKARI_CHECK_GL(glBindBuffer(MapTypeToTarget(_type), _handle));
}

void BufferOpenGL::UpdateData(GLintptr offset, GLsizei size, const void* data) const {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glNamedBufferSubData(GetHandle(), offset, size, data));
  } else {
    Bind();
    HIKARI_CHECK_GL(glBufferSubData(MapTypeToTarget(GetType()), offset, size, data));
  }
}

void BufferOpenGL::Store(const void* data, size_t size) {
  if (_type == BufferType::Unknown) {
    throw OpenGLException("unknown buffer");
  }
  if (IsValid()) {
    throw OpenGLException(std::string("buffer has stored data. handle id:") + std::to_string(_handle));
  }
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glCreateBuffers(1, &_handle));
  } else {
    HIKARI_CHECK_GL(glGenBuffers(1, &_handle));
  }
  GLbitfield flags = MapToBitField(_usage, _access);
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glNamedBufferStorage(_handle, static_cast<GLsizeiptr>(size), data, flags));
  } else {
    auto target = MapTypeToTarget(_type);
    HIKARI_CHECK_GL(glBindBuffer(target, _handle));
    if (feature.CanUseBufferStorage()) {
      HIKARI_CHECK_GL(glBufferStorage(target, static_cast<GLsizeiptr>(size), data, flags));
    } else {
      GLenum usageEnum = MapToUsage(_usage, _access);
      HIKARI_CHECK_GL(glBufferData(target, static_cast<GLsizeiptr>(size), data, usageEnum));
    }
  }
}

GLenum BufferOpenGL::MapTypeToTarget(BufferType type) {
  switch (type) {
    case BufferType::VertexBuffer:
      return GL_ARRAY_BUFFER;
    case BufferType::IndexBuffer:
      return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::UniformBuffer:
      return GL_UNIFORM_BUFFER;
    default:
      throw OpenGLException(std::string("unknown buffer type:") + std::to_string((int)type));
  }
}

GLbitfield BufferOpenGL::MapToBitField(BufferUsage usage, BufferAccess access) {
  GLbitfield flags = 0;
  if (usage == BufferUsage::Dynamic) {
    flags |= GL_DYNAMIC_STORAGE_BIT;
  }
  switch (access) {
    case BufferAccess::MapReadOnly:
      flags |= GL_MAP_READ_BIT;
      break;
    case BufferAccess::MapWriteOnly:
      flags |= GL_MAP_WRITE_BIT;
      break;
    case BufferAccess::MapReadWrite:
      flags |= GL_MAP_READ_BIT;
      flags |= GL_MAP_WRITE_BIT;
      break;
    default:
      break;
  }
  return flags;
}

GLenum BufferOpenGL::MapToUsage(BufferUsage usage, BufferAccess access) {
  if (usage == BufferUsage::Static) {
    if (access == BufferAccess::MapReadOnly) {
      return GL_STATIC_READ;
    } else {
      return GL_STATIC_DRAW;
    }
  } else {
    if (access == BufferAccess::MapReadOnly) {
      return GL_DYNAMIC_READ;
    } else {
      return GL_DYNAMIC_DRAW;
    }
  }
}

ShaderOpenGL::ShaderOpenGL() noexcept = default;

ShaderOpenGL::ShaderOpenGL(ShaderType type, const std::string& source) {
  _type = type;
  auto result = CompileFromSource(MapType(_type), source, _handle);
  if (!result) {
    throw OpenGLException("can't compile shader");
  }
}

ShaderOpenGL::ShaderOpenGL(ShaderOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
}

ShaderOpenGL& ShaderOpenGL::operator=(ShaderOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
  return *this;
}

ShaderOpenGL::~ShaderOpenGL() noexcept {
  Delete();
}

bool ShaderOpenGL::IsValid() const {
  return _handle != 0;
}

void ShaderOpenGL::Destroy() {
  Delete();
}

void ShaderOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteShader(_handle));
    _handle = 0;
  }
}

GLuint ShaderOpenGL::GetHandle() const {
  return _handle;
}

ShaderType ShaderOpenGL::GetType() const {
  return _type;
}

GLenum ShaderOpenGL::MapType(ShaderType type) {
  switch (type) {
    case ShaderType::Vertex:
      return GL_VERTEX_SHADER;
    case ShaderType::Fragment:
      return GL_FRAGMENT_SHADER;
    default:
      throw OpenGLException("unknown ShaderType");
  }
}

bool ShaderOpenGL::CompileFromSource(GLenum type, const std::string& source, GLuint& result) {
  if (source.empty()) {
    return false;
  }
  auto id = HIKARI_CHECK_GL(glCreateShader(type));
  auto sourcePtr = source.c_str();
  auto sourceLen = static_cast<GLint>(source.length());
  HIKARI_CHECK_GL(glShaderSource(id, 1, &sourcePtr, &sourceLen));
  HIKARI_CHECK_GL(glCompileShader(id));
  GLint status;
  HIKARI_CHECK_GL(glGetShaderiv(id, GL_COMPILE_STATUS, &status));
  bool isSuccess = status == GL_TRUE;
  if (isSuccess) {
    result = id;
  } else {
    int errorLen;
    HIKARI_CHECK_GL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &errorLen));
    auto errorInfo = std::make_unique<char[]>(errorLen);
    HIKARI_CHECK_GL(glGetShaderInfoLog(id, errorLen, nullptr, errorInfo.get()));
    std::cerr << "Shader Compile Error:\n"
              << errorInfo;
    HIKARI_CHECK_GL(glDeleteShader(id));
  }
  return isSuccess;
}

ShaderAttributeLayout::ShaderAttributeLayout() noexcept = default;

ShaderAttributeLayout::ShaderAttributeLayout(const std::string& name,
                                             SemanticType type,
                                             int index) noexcept {
  Name = name;
  Semantic = AttributeSemantic{type, index};
}

bool ShaderUniformBlock::Member::operator==(const ShaderUniformBlock::Member& o) const {
  return Name == o.Name && Location == o.Location && Type == o.Type && Length == o.Length && Offset == o.Offset;
}

bool ShaderUniformBlock::Member::operator!=(const ShaderUniformBlock::Member& o) const {
  return Name != o.Name || Location != o.Location || Type != o.Type || Length != o.Length || Offset != o.Offset;
}

bool ShaderUniformBlock::operator==(const ShaderUniformBlock& o) const {
  return Name == o.Name && Index == o.Index && DataSize == o.DataSize && Members == o.Members;
}

bool ShaderUniformBlock::operator!=(const ShaderUniformBlock& o) const {
  return Name != o.Name || Index != o.Index || DataSize != o.DataSize || Members != o.Members;
}

ProgramOpenGL::ProgramOpenGL() noexcept = default;

ProgramOpenGL::ProgramOpenGL(const ShaderOpenGL& vs,
                             const ShaderOpenGL& fs,
                             const ShaderAttributeLayouts& desc) {
  auto result = Link(vs, fs, desc, *this);
  if (!result) {
    throw OpenGLException(std::string("can't link program ") + std::to_string(_handle));
  }
}

ProgramOpenGL::ProgramOpenGL(ProgramOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _attribs = std::move(other._attribs);
  _uniforms = std::move(other._uniforms);
  _blocks = std::move(other._blocks);
  _nameToAttrib = std::move(other._nameToAttrib);
  _semanticToAttrib = std::move(other._semanticToAttrib);
  _nameToUni = std::move(other._nameToUni);
}

ProgramOpenGL& ProgramOpenGL::operator=(ProgramOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _attribs = std::move(other._attribs);
  _uniforms = std::move(other._uniforms);
  _blocks = std::move(other._blocks);
  _nameToAttrib = std::move(other._nameToAttrib);
  _semanticToAttrib = std::move(other._semanticToAttrib);
  _nameToUni = std::move(other._nameToUni);
  return *this;
}

ProgramOpenGL::~ProgramOpenGL() noexcept {
  Delete();
}

bool ProgramOpenGL::IsValid() const {
  return _handle != 0;
}

void ProgramOpenGL::Destroy() {
  Delete();
}

void ProgramOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteProgram(_handle));
    _handle = 0;
  }
}

VertexBufferBinding::VertexBufferBinding() noexcept = default;

VertexBufferBinding::VertexBufferBinding(GLuint point, GLuint handle, GLintptr offset, GLsizei stride) noexcept {
  BindingPoint = point;
  Handle = handle;
  Type = BufferType::VertexBuffer;
  Offset = offset;
  Stride = stride;
}

VertexBufferBinding::VertexBufferBinding(GLuint handle, GLintptr offset, GLsizei stride) noexcept {
  Handle = handle;
  Type = BufferType::IndexBuffer;
  Offset = offset;
  Stride = stride;
}

GLuint ProgramOpenGL::GetHandle() const {
  return _handle;
}

void ProgramOpenGL::Bind() const {
  HIKARI_CHECK_GL(glUseProgram(_handle));
}

std::optional<const ShaderAttribute*> ProgramOpenGL::GetAttribute(const std::string& name) const {
  auto iter = _nameToAttrib.find(name);
  if (iter == _nameToAttrib.end()) {
    return std::nullopt;
  } else {
    return std::make_optional(&_attribs[iter->second]);
  }
}

std::optional<const ShaderAttribute*> ProgramOpenGL::GetAttribute(AttributeSemantic semantic) const {
  auto iter = _semanticToAttrib.find(semantic);
  if (iter == _semanticToAttrib.end()) {
    return std::nullopt;
  } else {
    return std::make_optional(&_attribs[iter->second]);
  }
}

int ProgramOpenGL::GetBindingPoint(AttributeSemantic semantic) const {
  auto iter = _semanticToAttrib.find(semantic);
  if (iter == _semanticToAttrib.end()) {
    throw OpenGLException("can't find semantic");
  } else {
    return _attribs[iter->second].Location;
  }
}

GLuint ProgramOpenGL::GetAttributeLocation(const std::string& name) const {
  auto attrib = GetAttribute(name);
  if (attrib.has_value()) {
    return static_cast<GLuint>((*attrib)->Location);
  } else {
    throw OpenGLException(std::string("unknown attrubute ") + name);
  }
}

GLuint ProgramOpenGL::GetAttributeLocation(AttributeSemantic semantic) const {
  auto attrib = GetAttribute(semantic);
  if (attrib.has_value()) {
    return (*attrib)->Location;
  } else {
    throw OpenGLException("unknown attrubute");
  }
}

std::optional<ShaderUniform> ProgramOpenGL::TryGetUniform(const std::string& name) const {
  auto iter = _nameToUni.find(name);
  if (iter == _nameToUni.end()) {
    return std::nullopt;
  } else {
    return std::make_optional(_uniforms[iter->second]);
  }
}

const ShaderUniform& ProgramOpenGL::GetUniform(const std::string& name) const {
  auto iter = _nameToUni.find(name);
  if (iter == _nameToUni.end()) {
    throw OpenGLException("unknown uniform name");
  } else {
    return _uniforms[iter->second];
  }
}

void ProgramOpenGL::SubmitUniform(GLuint prog, GLint location, ParamType type, GLsizei count, const void* value) {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    switch (type) {
      case ParamType::Int32: {
        HIKARI_CHECK_GL(glProgramUniform1iv(prog, location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec2: {
        HIKARI_CHECK_GL(glProgramUniform2iv(prog, location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec3: {
        HIKARI_CHECK_GL(glProgramUniform3iv(prog, location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec4: {
        HIKARI_CHECK_GL(glProgramUniform4iv(prog, location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Float32: {
        HIKARI_CHECK_GL(glProgramUniform1fv(prog, location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec2: {
        HIKARI_CHECK_GL(glProgramUniform2fv(prog, location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec3: {
        HIKARI_CHECK_GL(glProgramUniform3fv(prog, location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec4: {
        HIKARI_CHECK_GL(glProgramUniform4fv(prog, location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat2: {
        HIKARI_CHECK_GL(glProgramUniformMatrix2fv(prog, location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat3: {
        HIKARI_CHECK_GL(glProgramUniformMatrix3fv(prog, location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat4: {
        HIKARI_CHECK_GL(glProgramUniformMatrix4fv(prog, location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Sampler2d:
      case ParamType::SamplerCubeMap: {
        HIKARI_CHECK_GL(glProgramUniform1iv(prog, location, count, static_cast<const GLint*>(value)));
        break;
      }
      default: {
        throw OpenGLException("unsupported uniform type");
      }
    }
  } else {
    switch (type) {
      case ParamType::Int32: {
        HIKARI_CHECK_GL(glUniform1iv(location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec2: {
        HIKARI_CHECK_GL(glUniform2iv(location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec3: {
        HIKARI_CHECK_GL(glUniform3iv(location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Int32Vec4: {
        HIKARI_CHECK_GL(glUniform4iv(location, count, static_cast<const GLint*>(value)));
        break;
      }
      case ParamType::Float32: {
        HIKARI_CHECK_GL(glUniform1fv(location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec2: {
        HIKARI_CHECK_GL(glUniform2fv(location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec3: {
        HIKARI_CHECK_GL(glUniform3fv(location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Vec4: {
        HIKARI_CHECK_GL(glUniform4fv(location, count, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat2: {
        HIKARI_CHECK_GL(glUniformMatrix2fv(location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat3: {
        HIKARI_CHECK_GL(glUniformMatrix3fv(location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Float32Mat4: {
        HIKARI_CHECK_GL(glUniformMatrix4fv(location, count, GL_FALSE, static_cast<const GLfloat*>(value)));
        break;
      }
      case ParamType::Sampler2d:
      case ParamType::SamplerCubeMap: {
        HIKARI_CHECK_GL(glUniform1iv(location, count, static_cast<const GLint*>(value)));
        break;
      }
      default: {
        throw OpenGLException("unsupported uniform type");
      }
    }
  }
}

void ProgramOpenGL::SubmitUniformMat4(GLuint prog, GLint index, const float* value) { SubmitUniform(prog, index, ParamType::Float32Mat4, 1, value); }
void ProgramOpenGL::SubmitUniformFloat(GLuint prog, GLint index, float value) { SubmitUniform(prog, index, ParamType::Float32, 1, &value); }
void ProgramOpenGL::SubmitUniformInt(GLuint prog, GLint index, int value) { SubmitUniform(prog, index, ParamType::Int32, 1, &value); }
void ProgramOpenGL::SubmitUniformVec3(GLuint prog, GLint index, const float* value) { SubmitUniform(prog, index, ParamType::Float32Vec3, 1, value); }
void ProgramOpenGL::SubmitUniformTex2d(GLuint prog, GLint index, GLuint handle) { SubmitUniform(prog, index, ParamType::Sampler2d, 1, &handle); }
void ProgramOpenGL::SubmitUniformCubeMap(GLuint prog, GLint index, GLuint handle) { SubmitUniform(prog, index, ParamType::SamplerCubeMap, 1, &handle); }
void ProgramOpenGL::UniformMat4(const std::string& name, const float* value) const { IfPresentUniform<const float*>(name, value, SubmitUniformMat4); }

void ProgramOpenGL::UniformFloat(const std::string& name, float value) const { IfPresentUniform<float>(name, value, SubmitUniformFloat); }
void ProgramOpenGL::UniformInt(const std::string& name, int value) const { IfPresentUniform<int>(name, value, SubmitUniformInt); }
void ProgramOpenGL::UniformVec3(const std::string& name, const float* value) const { IfPresentUniform<const float*>(name, value, SubmitUniformVec3); }
void ProgramOpenGL::UniformTexture2D(const std::string& name, GLuint handle) const { IfPresentUniform<GLuint>(name, handle, SubmitUniformTex2d); }
void ProgramOpenGL::UniformCubeMap(const std::string& name, GLuint handle) const { IfPresentUniform<GLuint>(name, handle, SubmitUniformCubeMap); }

ParamType ProgramOpenGL::MapType(GLenum type) {
  switch (type) {
    case GL_FLOAT:
      return ParamType::Float32;
    case GL_FLOAT_VEC2:
      return ParamType::Float32Vec2;
    case GL_FLOAT_VEC3:
      return ParamType::Float32Vec3;
    case GL_FLOAT_VEC4:
      return ParamType::Float32Vec4;
    case GL_FLOAT_MAT2:
      return ParamType::Float32Mat2;
    case GL_FLOAT_MAT3:
      return ParamType::Float32Mat3;
    case GL_FLOAT_MAT4:
      return ParamType::Float32Mat4;
    case GL_INT:
      return ParamType::Int32;
    case GL_INT_VEC2:
      return ParamType::Int32Vec2;
    case GL_INT_VEC3:
      return ParamType::Int32Vec3;
    case GL_INT_VEC4:
      return ParamType::Int32Vec4;
    case GL_SAMPLER_2D:
      return ParamType::Sampler2d;
    case GL_SAMPLER_CUBE:
      return ParamType::SamplerCubeMap;
    default:
      return ParamType::Unknown;
  }
}

size_t ProgramOpenGL::MapParamSize(ParamType type) {
  switch (type) {
    case Hikari::ParamType::Int32:
      return sizeof(int32_t);
    case Hikari::ParamType::Int32Vec2:
      return sizeof(int32_t) * 2;
    case Hikari::ParamType::Int32Vec3:
      return sizeof(int32_t) * 3;
    case Hikari::ParamType::Int32Vec4:
      return sizeof(int32_t) * 4;
    case Hikari::ParamType::Float32:
      return sizeof(float_t);
    case Hikari::ParamType::Float32Vec2:
      return sizeof(float_t) * 2;
    case Hikari::ParamType::Float32Vec3:
      return sizeof(float_t) * 3;
    case Hikari::ParamType::Float32Vec4:
      return sizeof(float_t) * 4;
    case Hikari::ParamType::Float32Mat2:
      return sizeof(float_t) * 2 * 2;
    case Hikari::ParamType::Float32Mat3:
      return sizeof(float_t) * 3 * 3;
    case Hikari::ParamType::Float32Mat4:
      return sizeof(float_t) * 4 * 4;
    case Hikari::ParamType::Sampler2d:
      return sizeof(GLuint);
    case Hikari::ParamType::SamplerCubeMap:
      return sizeof(GLuint);
    default:
      throw OpenGLException("out of range");
  }
}

bool ProgramOpenGL::Link(const ShaderOpenGL& vs,
                         const ShaderOpenGL& fs,
                         const ShaderAttributeLayouts& desc,
                         ProgramOpenGL& result) {
  auto id = HIKARI_CHECK_GL(glCreateProgram());
  HIKARI_CHECK_GL(glAttachShader(id, vs.GetHandle()));
  HIKARI_CHECK_GL(glAttachShader(id, fs.GetHandle()));
  HIKARI_CHECK_GL(glLinkProgram(id));
  GLint status;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_LINK_STATUS, &status));
  if (status == GL_TRUE) {
    result._handle = id;
    auto activeAttrib = ReflectActiveAttrib(result.GetHandle());
    result._attribs.reserve(activeAttrib.size());
    for (const auto& aInfo : activeAttrib) {
      auto ad = std::find_if(desc.begin(),
                             desc.end(),
                             [&](const auto& d) { return d.Name == aInfo.Name; });
      AttributeSemantic semantic{};
      if (ad == desc.end()) {
        std::cout << "Unknown semantic of attribute " << aInfo.Name << '\n';
      } else {
        semantic = ad->Semantic;
      }
      ShaderAttribute attrib;
      attrib.Name = aInfo.Name;
      attrib.Type = aInfo.Type;
      attrib.Length = aInfo.Length;
      attrib.Location = aInfo.Location;
      attrib.Semantic = semantic;
      result._attribs.emplace_back(attrib);
    }
    result._nameToAttrib.reserve(activeAttrib.size());
    result._semanticToAttrib.reserve(activeAttrib.size());
    for (size_t i = 0; i < result._attribs.size(); i++) {
      const auto& attrib = result._attribs[i];
      auto [it0, isNameInsert] = result._nameToAttrib.emplace(attrib.Name, i);
      auto [it1, isSemInsert] = result._semanticToAttrib.emplace(attrib.Semantic, i);
      assert(isNameInsert);
      assert(isSemInsert);
    }
    result._uniforms = ReflectActiveUniform(result.GetHandle());
    result._nameToUni.reserve(result._uniforms.size());
    for (size_t i = 0; i < result._uniforms.size(); i++) {
      auto [_, isNameInsert] = result._nameToUni.emplace(result._uniforms[i].Name, i);
      assert(isNameInsert);
    }
    result._blocks = ReflectActiveBlock(result.GetHandle());
    return true;
  } else {
    int errorLen;
    HIKARI_CHECK_GL(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &errorLen));
    auto errorInfo = std::make_unique<char[]>(errorLen);
    HIKARI_CHECK_GL(glGetProgramInfoLog(id, errorLen, nullptr, errorInfo.get()));
    std::cerr << "ProgramOpenGL::Link():" << errorInfo << '\n';
    HIKARI_CHECK_GL(glDeleteProgram(id));
    return false;
  }
}

std::vector<ShaderAttribute> ProgramOpenGL::ReflectActiveAttrib(GLuint prog) {
  auto id = prog;
  GLint attribCount;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attribCount));
  std::vector<ShaderAttribute> result;
  result.reserve(attribCount);
  GLint maxNameLen;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLen));
  auto attrName = std::make_unique<char[]>(maxNameLen);
  for (int i = 0; i < attribCount; i++) {
    GLenum type = 0;
    GLint size = 0;
    GLsizei nameSize = 0;
    HIKARI_CHECK_GL(glGetActiveAttrib(id, i, maxNameLen, &nameSize, &size, &type, attrName.get()));
    GLint location = HIKARI_CHECK_GL(glGetAttribLocation(id, attrName.get()));
    ShaderAttribute desc;
    desc.Location = location;
    desc.Length = size;
    desc.Name = std::string(attrName.get(), nameSize);
    desc.Type = MapType(type);
    desc.Semantic = AttributeSemantic(SemanticType::Unknown, 0);
    assert(desc.Type != ParamType::Unknown);
    assert(desc.Length >= 1);
    assert(desc.Location >= 0);
    result.emplace_back(desc);
  }
  return result;
}

std::vector<ShaderUniform> ProgramOpenGL::ReflectActiveUniform(GLuint prog) {
  auto id = prog;
  GLint uniformCount;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount));
  std::vector<ShaderUniform> result;
  result.reserve(uniformCount);
  GLint maxNameLen = 0;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLen));
  auto uniName = std::make_unique<char[]>(maxNameLen);
  for (int i = 0; i < uniformCount; i++) {
    GLenum type = 0;
    GLint size = 0;
    GLsizei nameSize = 0;
    HIKARI_CHECK_GL(glGetActiveUniform(id, i, maxNameLen, &nameSize, &size, &type, uniName.get()));
    GLint index = HIKARI_CHECK_GL(glGetUniformLocation(id, uniName.get()));
    if (index < 0) {
      continue;
    }
    ShaderUniform desc;
    desc.Location = index;
    desc.Length = size;
    desc.Name = std::string(uniName.get(), nameSize);
    desc.Type = MapType(type);
    if (desc.Type == ParamType::Unknown) {
      throw OpenGLException("unknown Type");
    }
    if (desc.Length < 1) {
      throw OpenGLException("invalid Length");
    }
    result.emplace_back(desc);
  }
  return result;
}

std::vector<ShaderUniformBlock> ProgramOpenGL::ReflectActiveBlock(GLuint prog) {
  auto id = prog;
  GLint blockCount;
  HIKARI_CHECK_GL(glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount));
  std::vector<ShaderUniformBlock> result;
  result.reserve(blockCount);
  for (int i = 0; i < blockCount; i++) {
    int nameLen;
    HIKARI_CHECK_GL(glGetActiveUniformBlockiv(id, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen));
    int uniCnt;
    HIKARI_CHECK_GL(glGetActiveUniformBlockiv(id, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniCnt));
    int dataSize;
    HIKARI_CHECK_GL(glGetActiveUniformBlockiv(id, i, GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize));
    auto name = std::make_unique<char[]>(nameLen);
    HIKARI_CHECK_GL(glGetActiveUniformBlockName(id, i, nameLen, nullptr, name.get()));
    int index = HIKARI_CHECK_GL(glGetUniformBlockIndex(prog, name.get()));

    auto indices = std::make_unique<int[]>(uniCnt);
    auto indicesPtr = reinterpret_cast<uint32_t*>(indices.get());
    HIKARI_CHECK_GL(glGetActiveUniformBlockiv(id, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.get()));
    auto nameLens = std::make_unique<int[]>(uniCnt);
    HIKARI_CHECK_GL(glGetActiveUniformsiv(id, uniCnt, indicesPtr, GL_UNIFORM_NAME_LENGTH, nameLens.get()));
    auto types = std::make_unique<int[]>(uniCnt);
    HIKARI_CHECK_GL(glGetActiveUniformsiv(id, uniCnt, indicesPtr, GL_UNIFORM_TYPE, types.get()));
    auto sizes = std::make_unique<int[]>(uniCnt);
    HIKARI_CHECK_GL(glGetActiveUniformsiv(id, uniCnt, indicesPtr, GL_UNIFORM_SIZE, sizes.get()));
    auto offsets = std::make_unique<int[]>(uniCnt);
    HIKARI_CHECK_GL(glGetActiveUniformsiv(id, uniCnt, indicesPtr, GL_UNIFORM_OFFSET, offsets.get()));
    std::vector<ShaderUniformBlock::Member> members(uniCnt);
    for (int j = 0; j < uniCnt; j++) {
      auto memName = std::make_unique<char[]>(nameLens[j]);
      HIKARI_CHECK_GL(glGetActiveUniformName(id, indices[j], nameLens[j], nullptr, memName.get()));
      members[j].Name = std::string(memName.get(), nameLens[j]);
      members[j].Location = indices[j];
      members[j].Type = MapType(types[j]);
      members[j].Length = sizes[j];
      members[j].Offset = offsets[j];
    }

    ShaderUniformBlock block;
    block.Name = std::string(name.get());
    block.Index = index;
    block.DataSize = dataSize;
    block.Members = std::move(members);

    result.emplace_back(block);
  }
  return result;
}

VertexArrayOpenGL::VertexArrayOpenGL() noexcept = default;

VertexArrayOpenGL::VertexArrayOpenGL(const VertexBufferBindings& bufferBindings,
                                     const VertexAttributeFormats& attribFormats,
                                     const VertexAssociates& associates) {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseVertexAttribBinding()) {
    /*
    GL_ARB_vertex_attrib_binding将GL3的glVertexAttribPointer拆成了两部分，并引入了绑定点这一概念(Binding Point)
    glBindVertexBuffer用来将Buffer绑定到Binding Point
    glVertexAttribFormat用来定义顶点属性
    glVertexAttribBinding用来连接顶点属性和绑定点，这样就将顶点属性和Buffer间接关联起来了
    */
    if (feature.CanUseDirectStateAccess()) {
      HIKARI_CHECK_GL(glCreateVertexArrays(1, &_handle));
      for (const auto& binding : bufferBindings) {
        if (binding.Type == BufferType::VertexBuffer) {
          HIKARI_CHECK_GL(glVertexArrayVertexBuffer(_handle,
                                                    binding.BindingPoint,
                                                    binding.Handle,
                                                    binding.Offset,
                                                    binding.Stride));
        } else if (binding.Type == BufferType::IndexBuffer) {
          HIKARI_CHECK_GL(glVertexArrayElementBuffer(_handle, binding.Handle));
        }
      }
      for (const auto& format : attribFormats) {
        auto [size, type] = MapType(format.Type);
        HIKARI_CHECK_GL(glVertexArrayAttribFormat(_handle,
                                                  format.AttribIndex,
                                                  size, type,
                                                  format.IsNormalised,
                                                  format.RelativeOffset));
      }
      for (const auto& associate : associates) {
        HIKARI_CHECK_GL(glVertexArrayAttribBinding(_handle, associate.AttribIndex, associate.BindingPoint));
      }
      for (const auto& format : attribFormats) {
        HIKARI_CHECK_GL(glEnableVertexArrayAttrib(_handle, format.AttribIndex));
      }
    } else {
      HIKARI_CHECK_GL(glGenVertexArrays(1, &_handle));
      HIKARI_CHECK_GL(glBindVertexArray(_handle));
      for (const auto& binding : bufferBindings) {
        if (binding.Type == BufferType::VertexBuffer) {
          HIKARI_CHECK_GL(glBindVertexBuffer(binding.BindingPoint,
                                             binding.Handle,
                                             binding.Offset,
                                             binding.Stride));
        } else if (binding.Type == BufferType::IndexBuffer) {
          HIKARI_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, binding.Handle));
        }
      }
      for (const auto& format : attribFormats) {
        auto [size, type] = MapType(format.Type);
        HIKARI_CHECK_GL(glVertexAttribFormat(format.AttribIndex,
                                             size, type,
                                             format.IsNormalised,
                                             format.RelativeOffset));
      }
      for (const auto& associate : associates) {
        HIKARI_CHECK_GL(glVertexAttribBinding(associate.AttribIndex, associate.BindingPoint));
      }
      for (const auto& format : attribFormats) {
        HIKARI_CHECK_GL(glEnableVertexAttribArray(format.AttribIndex));
      }
    }
  } else {
    //手动模拟绑定点:(
    HIKARI_CHECK_GL(glGenVertexArrays(1, &_handle));
    HIKARI_CHECK_GL(glBindVertexArray(_handle));
    std::unordered_map<GLuint, VertexBufferBinding> buf;
    std::unordered_map<GLuint, VertexAttributeFormat> att;
    std::vector<VertexBufferBinding> ibos;
    for (const auto& binding : bufferBindings) {
      if (binding.Type == BufferType::VertexBuffer) {
        buf.emplace(binding.BindingPoint, binding);
      } else if (binding.Type == BufferType::IndexBuffer) {
        ibos.emplace_back(binding);
      }
    }
    for (const auto& format : attribFormats) {
      att.emplace(format.AttribIndex, format);
    }
    for (const auto& associate : associates) {
      const auto& binding = buf[associate.BindingPoint];
      const auto& format = att[associate.AttribIndex];
      auto [size, type] = MapType(format.Type);
      auto target = BufferOpenGL::MapTypeToTarget(binding.Type);
      if (binding.Type == BufferType::VertexBuffer) {
        HIKARI_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, binding.Handle));
        HIKARI_CHECK_GL(glVertexAttribPointer(format.AttribIndex,
                                              size, type,
                                              format.IsNormalised,
                                              binding.Stride,  //两组数据之间间隔
                                              //buffer内偏移量+绑定点相对偏移量
                                              (void*)(binding.Offset + format.RelativeOffset)));
        HIKARI_CHECK_GL(glEnableVertexAttribArray(format.AttribIndex));
      }
    }
    for (const auto& ibo : ibos) {
      HIKARI_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.Handle));
    }
    HIKARI_CHECK_GL(glBindVertexArray(0));  //如果不绑默认vao，万一后面还有buffer绑定行为就芜湖了
  }
}

VertexArrayOpenGL::VertexArrayOpenGL(const std::vector<ShaderAttribute>& attribs) {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glCreateVertexArrays(1, &_handle));
    for (const auto& attrib : attribs) {
      auto [size, type] = MapType(attrib.Type);
      HIKARI_CHECK_GL(glVertexArrayAttribFormat(_handle, attrib.Location, size, type, GL_FALSE, 0));
      HIKARI_CHECK_GL(glVertexArrayAttribBinding(_handle, attrib.Location, attrib.Location));
      HIKARI_CHECK_GL(glEnableVertexArrayAttrib(_handle, attrib.Location));
    }
  } else {
    HIKARI_CHECK_GL(glGenVertexArrays(1, &_handle));
    HIKARI_CHECK_GL(glBindVertexArray(_handle));
    if (feature.CanUseVertexAttribBinding()) {
      for (const auto& attrib : attribs) {
        auto [size, type] = MapType(attrib.Type);
        HIKARI_CHECK_GL(glVertexAttribFormat(attrib.Location, size, type, GL_FALSE, 0));
        HIKARI_CHECK_GL(glVertexAttribBinding(attrib.Location, attrib.Location));
        HIKARI_CHECK_GL(glEnableVertexAttribArray(attrib.Location));
      }
    } else {
      for (const auto& attrib : attribs) {
        _attribFormat.emplace(attrib.Location, VertexAttributeFormat{(GLuint)attrib.Location, attrib.Type, GL_FALSE, 0});
        HIKARI_CHECK_GL(glEnableVertexAttribArray(attrib.Location));
      }
    }
    HIKARI_CHECK_GL(glBindVertexArray(0));
  }
}

VertexArrayOpenGL::VertexArrayOpenGL(VertexArrayOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;

  _attribFormat = std::move(other._attribFormat);
}

VertexArrayOpenGL& VertexArrayOpenGL::operator=(VertexArrayOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;

  _attribFormat = std::move(other._attribFormat);
  return *this;
}

VertexArrayOpenGL::~VertexArrayOpenGL() noexcept {
  Delete();
}

bool VertexArrayOpenGL::IsValid() const {
  return _handle != 0;
}

void VertexArrayOpenGL::Destroy() {
  Delete();
}

void VertexArrayOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteVertexArrays(1, &_handle));
    _handle = 0;
  }
}

GLuint VertexArrayOpenGL::GetHandle() const {
  return _handle;
}

void VertexArrayOpenGL::Bind() const {
  HIKARI_CHECK_GL(glBindVertexArray(_handle));
}

void VertexArrayOpenGL::SetVertexBuffer(const VertexBufferBinding& binding) const {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {  //DSA
    HIKARI_CHECK_GL(glVertexArrayVertexBuffer(_handle,
                                              binding.BindingPoint,
                                              binding.Handle,
                                              binding.Offset,
                                              binding.Stride));
  } else {
    if (feature.CanUseVertexAttribBinding()) {  //Bingd
      HIKARI_CHECK_GL(glBindVertexBuffer(binding.BindingPoint, binding.Handle, binding.Offset, binding.Stride));
    } else {  //opengl version<=4.2
      const auto& format = _attribFormat.at(binding.BindingPoint);
      auto [size, type] = MapType(format.Type);
      HIKARI_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, binding.Handle));
      HIKARI_CHECK_GL(glVertexAttribPointer(format.AttribIndex,
                                            size, type,
                                            format.IsNormalised,
                                            binding.Stride,  //两组数据之间间隔
                                            //buffer内偏移量+绑定点相对偏移量
                                            (void*)(binding.Offset + format.RelativeOffset)));
    }
  }
}

void VertexArrayOpenGL::SetIndexBuffer(GLuint iboHandle) const {
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glVertexArrayElementBuffer(_handle, iboHandle));
  } else {
    HIKARI_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboHandle));
  }
}

std::pair<GLint, GLenum> VertexArrayOpenGL::MapType(ParamType type) {
  switch (type) {
    case ParamType::Int32:
      return std::make_pair<GLint, GLenum>(1, GL_INT);
    case ParamType::Int32Vec2:
      return std::make_pair<GLint, GLenum>(2, GL_INT);
    case ParamType::Int32Vec3:
      return std::make_pair<GLint, GLenum>(3, GL_INT);
    case ParamType::Int32Vec4:
      return std::make_pair<GLint, GLenum>(4, GL_INT);
    case ParamType::Float32:
      return std::make_pair<GLint, GLenum>(1, GL_FLOAT);
    case ParamType::Float32Vec2:
      return std::make_pair<GLint, GLenum>(2, GL_FLOAT);
    case ParamType::Float32Vec3:
      return std::make_pair<GLint, GLenum>(3, GL_FLOAT);
    case ParamType::Float32Vec4:
      return std::make_pair<GLint, GLenum>(4, GL_FLOAT);
    default:
      throw OpenGLException("unknown ParamType");
  }
}

TextureOpenGL::TextureOpenGL() noexcept = default;

TextureOpenGL::TextureOpenGL(const Texture2dDescriptorOpenGL& desc) {
  CreateTexture2d(desc, *this);
  _width = desc.Width;
  _height = desc.Height;
}

TextureOpenGL::TextureOpenGL(const TextureCubeMapDescriptorOpenGL& desc) {
  CreateCubeMap(desc, *this);
  _width = desc.Width;
  _height = desc.Height;
}

TextureOpenGL::TextureOpenGL(const DepthTextureDescriptorOpenGL& desc) {
  auto min = MapFilterMode(desc.MinFilter);
  auto mag = MapFilterMode(desc.MagFilter);
  auto wrap = MapWrapMode(desc.Wrap);
  auto texFormat = static_cast<GLenum>(desc.TextureFormat);
  auto dataFormat = GL_DEPTH_COMPONENT;
  auto dataType = MapTextureDataType(desc.DataType);
  auto width = desc.Width;
  auto height = desc.Height;
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    GLuint name;
    HIKARI_CHECK_GL(glCreateTextures(GL_TEXTURE_2D, 1, &name));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_T, wrap));
    //为啥加这两行就没法采样纹理了...
    //HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
    //HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_COMPARE_FUNC, GL_LESS));
    HIKARI_CHECK_GL(glTextureStorage2D(name, 1, texFormat, width, height));
    _handle = name;
  } else {
    HIKARI_CHECK_GL(glGenTextures(1, &_handle));
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_2D, _handle));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
    //HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
    //HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS));
    if (feature.CanUseTextureStorage()) {
      HIKARI_CHECK_GL(glTexStorage2D(GL_TEXTURE_2D, 1, texFormat, width, height));
    } else {
      HIKARI_CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, dataFormat, dataType, 0));
    }
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
  }
  _type = TextureType::Image2d;
  _width = desc.Width;
  _height = desc.Height;
}

void TextureOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteTextures(1, &_handle));
    _handle = 0;
  }
}

TextureOpenGL::TextureOpenGL(TextureOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
  _width = other._width;
  _height = other._height;
}

TextureOpenGL& TextureOpenGL::operator=(TextureOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  _type = other._type;
  _width = other._width;
  _height = other._height;
  return *this;
}

TextureOpenGL::~TextureOpenGL() noexcept {
  Delete();
}

void TextureOpenGL::Destroy() {
  Delete();
}

bool TextureOpenGL::IsValid() const {
  return _handle != 0 && _type != TextureType::Unknown;
}

GLuint TextureOpenGL::GetHandle() const {
  return _handle;
}

TextureType TextureOpenGL::GetType() const {
  return _type;
}

int TextureOpenGL::GetWidth() const { return _width; }

int TextureOpenGL::GetHeight() const { return _height; }

GLuint TextureOpenGL::MapFilterMode(FilterMode mode) {
  switch (mode) {
    case FilterMode::Point:
      return GL_NEAREST;
    case FilterMode::Bilinear:
      return GL_LINEAR;
    case FilterMode::Trilinear:
      return GL_LINEAR_MIPMAP_LINEAR;
    default:
      throw OpenGLException("unknown FilterMode");
  }
}

GLuint TextureOpenGL::MapWrapMode(WrapMode mode) {
  switch (mode) {
    case WrapMode::Repeat:
      return GL_REPEAT;
    case WrapMode::Clamp:
      return GL_CLAMP_TO_EDGE;
    case WrapMode::Mirror:
      return GL_MIRRORED_REPEAT;
    default:
      throw OpenGLException("unknown WrapMode");
  }
}

GLint TextureOpenGL::MapPixelFormat(ColorFormat format) {
  switch (format) {
    case ColorFormat::RGB:
      return GL_RGB;
    case ColorFormat::RGBA:
      return GL_RGBA;
    case ColorFormat::Depth:
      return GL_DEPTH_COMPONENT;
    default:
      throw OpenGLException("unknown ColorFormat");
  }
}

GLenum TextureOpenGL::MapTextureDataType(ImageDataType format) {
  switch (format) {
    case ImageDataType::Byte:
      return GL_UNSIGNED_BYTE;
    case ImageDataType::Float32:
      return GL_FLOAT;
    default:
      throw OpenGLException("unknown ImageDataType");
  }
}

GLsizei TextureOpenGL::CalcMipmapLevels(int mipmapLevel, int maxSize, bool isUseTrilinear) {
  int levels;
  if (isUseTrilinear) {  //三线性过滤才启用mipmap
    int maxLevel = (int)std::log2(maxSize) + 1;
    if (mipmapLevel == 0 || mipmapLevel > maxLevel) {
      levels = maxLevel;
    } else {
      levels = mipmapLevel;
    }
  } else {
    levels = 1;
  }
  return levels;
}

void TextureOpenGL::CreateTexture2d(const Texture2dDescriptorOpenGL& desc, TextureOpenGL& texture) {
  auto min = MapFilterMode(desc.MinFilter);
  auto mag = MapFilterMode(desc.MagFilter);
  auto wrap = MapWrapMode(desc.Wrap);
  auto texFormat = static_cast<GLenum>(desc.TextureFormat);
  auto dataFormat = (GLenum)MapPixelFormat(desc.DataFormat);
  auto dataType = MapTextureDataType(desc.DataType);
  auto width = desc.Width;
  auto height = desc.Height;
  auto levels = CalcMipmapLevels(desc.MipMapLevel, std::max(width, height), desc.MinFilter == FilterMode::Trilinear);
  const auto& feature = FeatureOpenGL::Get();
  if (feature.CanUseDirectStateAccess()) {
    GLuint name;
    HIKARI_CHECK_GL(glCreateTextures(GL_TEXTURE_2D, 1, &name));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_T, wrap));
    HIKARI_CHECK_GL(glTextureStorage2D(name, levels, texFormat, width, height));
    HIKARI_CHECK_GL(glTextureSubImage2D(name, 0, 0, 0, width, height, dataFormat, dataType, desc.DataPtr));
    HIKARI_CHECK_GL(glGenerateTextureMipmap(name));
    texture._handle = name;
  } else {
    HIKARI_CHECK_GL(glGenTextures(1, &texture._handle));
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_2D, texture._handle));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));
    if (feature.CanUseTextureStorage()) {
      HIKARI_CHECK_GL(glTexStorage2D(GL_TEXTURE_2D, levels, texFormat, width, height));
      HIKARI_CHECK_GL(glTextureSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, dataFormat, dataType, desc.DataPtr));
    } else {
      HIKARI_CHECK_GL(glTexImage2D(GL_TEXTURE_2D,
                                   0,
                                   texFormat,
                                   width, height,
                                   0,
                                   dataFormat, dataType, static_cast<GLvoid*>(desc.DataPtr)));
    }
    HIKARI_CHECK_GL(glGenerateMipmap(GL_TEXTURE_2D));
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
  }
  texture._type = TextureType::Image2d;
}

void TextureOpenGL::CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc, TextureOpenGL& texture) {
  const auto& feature = FeatureOpenGL::Get();
  auto min = MapFilterMode(desc.MinFilter);
  auto mag = MapFilterMode(desc.MagFilter);
  auto wrap = MapWrapMode(desc.Wrap);
  auto levels = CalcMipmapLevels(desc.MipMapLevel,
                                 std::max(desc.Width, desc.Height),
                                 desc.MinFilter == FilterMode::Trilinear);
  if (feature.CanUseDirectStateAccess()) {
    GLuint name;
    auto texFormat = static_cast<GLenum>(desc.TextureFormat);
    HIKARI_CHECK_GL(glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &name));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_T, wrap));
    HIKARI_CHECK_GL(glTextureParameteri(name, GL_TEXTURE_WRAP_R, wrap));
    HIKARI_CHECK_GL(glTextureStorage2D(name, levels, texFormat, desc.Width, desc.Height));
    for (int i = 0; i < 6; i++) {
      auto dataFormat = (GLenum)MapPixelFormat(desc.DataFormat[i]);
      auto dataType = MapTextureDataType(desc.DataType[i]);
      HIKARI_CHECK_GL(glTextureSubImage3D(name,
                                          0,        //mipmap的第n个级别
                                          0, 0, i,  //x,y,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
                                          desc.Width, desc.Height,
                                          1,  //一次性上传多少个cubmap纹理,这里一个一个传
                                          dataFormat, dataType, desc.DataPtr[i]));
    }
    HIKARI_CHECK_GL(glGenerateTextureMipmap(name));
    texture._handle = name;
  } else {
    HIKARI_CHECK_GL(glGenTextures(1, &texture._handle));
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, texture._handle));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap));
    HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap));
    auto texFormat = static_cast<GLenum>(desc.TextureFormat);
    if (feature.CanUseTextureStorage()) {
      glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, texFormat, desc.Width, desc.Height);
      for (int i = 0; i < 6; i++) {
        auto dataFormat = (GLenum)MapPixelFormat(desc.DataFormat[i]);
        auto dataType = MapTextureDataType(desc.DataType[i]);
        HIKARI_CHECK_GL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                                        0, 0,
                                        desc.Width, desc.Height,
                                        dataFormat, dataType, desc.DataPtr[i]));
      }
    } else {
      //max level从0开始
      HIKARI_CHECK_GL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, levels - 1));
      for (int i = 0; i < 6; i++) {
        auto dataFormat = (GLenum)MapPixelFormat(desc.DataFormat[i]);
        auto dataType = MapTextureDataType(desc.DataType[i]);
        HIKARI_CHECK_GL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                     0,  //mipmap的第n个级别
                                     texFormat,
                                     desc.Width, desc.Height,
                                     0,  //*永远* 是0
                                     dataFormat, dataType, static_cast<GLvoid*>(desc.DataPtr[i])));
      }
    }
    HIKARI_CHECK_GL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
    HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
  }
  texture._type = TextureType::CubeMap;
}

GLenum TextureOpenGL::MapTextureType(TextureType type) {
  switch (type) {
    case Hikari::TextureType::Image2d:
      return GL_TEXTURE_2D;
    case Hikari::TextureType::CubeMap:
      return GL_TEXTURE_CUBE_MAP;
    default:
      throw OpenGLException("unknown TextureType");
  }
}

FrameBufferOpenGL::FrameBufferOpenGL() noexcept = default;

FrameBufferOpenGL::FrameBufferOpenGL(const FrameBufferDepthDescriptor& depth) {
  const auto& feature = FeatureOpenGL::Get();
  GLenum status{};
  if (feature.CanUseDirectStateAccess()) {
    HIKARI_CHECK_GL(glCreateFramebuffers(1, &_handle));
    HIKARI_CHECK_GL(glNamedFramebufferDrawBuffer(_handle, GL_NONE));
    HIKARI_CHECK_GL(glNamedFramebufferReadBuffer(_handle, GL_NONE));
    HIKARI_CHECK_GL(glNamedFramebufferTexture(_handle, GL_DEPTH_ATTACHMENT, depth.Texture2D, 0));
    status = HIKARI_CHECK_GL(glCheckNamedFramebufferStatus(_handle, GL_FRAMEBUFFER));
  } else {
    HIKARI_CHECK_GL(glGenFramebuffers(1, &_handle));
    HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, _handle));
    HIKARI_CHECK_GL(glDrawBuffer(GL_NONE));
    HIKARI_CHECK_GL(glReadBuffer(GL_NONE));
    HIKARI_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth.Texture2D, 0));
    status = HIKARI_CHECK_GL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  }
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    throw OpenGLException(std::string("can't create depth framebuffer code:") + std::to_string(status));
  }
}

FrameBufferOpenGL::FrameBufferOpenGL(FrameBufferOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
}

FrameBufferOpenGL& FrameBufferOpenGL::operator=(FrameBufferOpenGL&& other) noexcept {
  _handle = other._handle;
  other._handle = 0;
  return *this;
}

FrameBufferOpenGL::~FrameBufferOpenGL() noexcept {
  Delete();
}

void FrameBufferOpenGL::Destroy() {
  Delete();
}

void FrameBufferOpenGL::Bind() const {
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, _handle));
}

void FrameBufferOpenGL::Unbind() const {
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBufferOpenGL::Delete() {
  if (_handle != 0) {
    HIKARI_CHECK_GL(glDeleteFramebuffers(1, &_handle));
    _handle = 0;
  }
}

bool FrameBufferOpenGL::IsValid() const {
  return _handle != 0;
}

}  // namespace Hikari