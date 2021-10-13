#include <hikari/render_context.h>

#include <iostream>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <sstream>

#include <SPIRV/GlslangToSpv.h>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>

#include <hikari/asset.h>

namespace Hikari {
//从glslang里cv来的（
constexpr const TBuiltInResource __BuiltInRes = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

ShaderIncluder::ShaderIncluder() noexcept = default;

ShaderIncluder ::~ShaderIncluder() = default;

glslang::TShader::Includer::IncludeResult* ShaderIncluder::includeSystem(const char* headerName,
                                                                         const char* includerName,
                                                                         size_t inclusionDepth) {
  for (const auto& sysPath : _systemPaths) {
    auto findPath = sysPath / headerName;
    if (!std::filesystem::exists(findPath)) {
      continue;
    }
    try {
      auto str = ReadText(findPath);
      auto result = new char[str.length()];
      std::copy(str.begin(), str.end(), result);
      return new IncludeResult(headerName, result, str.length(), result);
    } catch (std::exception& e) {
      return new IncludeResult(std::string(), e.what(), strlen(e.what()), nullptr);
    }
  }
  return nullptr;
}

glslang::TShader::Includer::IncludeResult* ShaderIncluder::includeLocal(const char* headerName,
                                                                        const char* includerName,
                                                                        size_t inclusionDepth) {
  if (_workPath.empty()) {
    std::error_code errCode;
    _workPath = std::filesystem::current_path(errCode);
    if (errCode) {
      throw RenderContextException(errCode.message());
    }
  }
  auto findPath = _workPath / headerName;
  if (!std::filesystem::exists(findPath)) {
    return nullptr;
  }
  try {
    auto str = ReadText(findPath);
    auto result = new char[str.length()];
    std::copy(str.begin(), str.end(), result);
    return new IncludeResult(headerName, result, str.length(), result);
  } catch (std::exception& e) {
    return new IncludeResult(std::string(), e.what(), strlen(e.what()), nullptr);
  }
}

void ShaderIncluder::releaseInclude(IncludeResult* result) {
  if (result != nullptr) {
    if (result->userData != nullptr) {
      delete[] static_cast<char*>(result->userData);
    }
    delete result;
  }
}

void ShaderIncluder::AddSystemPath(const std::filesystem::path& sysPath) {
  auto iter = std::find(_systemPaths.begin(), _systemPaths.end(), sysPath);
  if (iter == _systemPaths.end()) {
    _systemPaths.emplace_back(sysPath);
  }
}

std::string ShaderIncluder::ReadText(const std::filesystem::path& p) {
  std::ifstream stream(p, std::ios_base::in | std::ios::binary);
  auto size = std::filesystem::file_size(p);
  std::string text(size, '\0');
  stream.read(text.data(), size);
  return text;
}

RenderContextOpenGL::RenderContextOpenGL() noexcept = default;

RenderContextOpenGL::RenderContextOpenGL(RenderContextOpenGL&& other) noexcept {
  _objects = std::move(other._objects);
  _vaos = std::move(other._vaos);
  _globalBlocks = std::move(other._globalBlocks);
  _blockQueryMap = std::move(other._blockQueryMap);
  _globalUniforms = std::move(other._globalUniforms);
  _isValid = other._isValid;
  other._isValid = false;
}

RenderContextOpenGL& RenderContextOpenGL::operator=(RenderContextOpenGL&& other) noexcept {
  _objects = std::move(other._objects);
  _vaos = std::move(other._vaos);
  _globalBlocks = std::move(other._globalBlocks);
  _blockQueryMap = std::move(other._blockQueryMap);
  _globalUniforms = std::move(other._globalUniforms);
  _isValid = other._isValid;
  other._isValid = false;
  return *this;
}

void RenderContextOpenGL::Destroy() {
  _globalBlocks.clear();
  _blockQueryMap.clear();
  _globalUniforms.clear();
  for (auto& [_, vao] : _vaos) {
    vao.Destroy();
  }
  _vaos.clear();
  for (const auto& obj : _objects) {
    obj->Destroy();
  }
  _objects.clear();
  _isValid = false;
}

std::shared_ptr<BufferOpenGL> RenderContextOpenGL::CreateBuffer(const void* data, size_t size, BufferType type,
                                                                BufferUsage usage, BufferAccess access) {
  CheckInit();
  auto buffer = std::make_shared<BufferOpenGL>(data, size, type, usage, access);
  if (!buffer->IsValid()) {
    throw RenderContextException("Can't create buffer");
  }
  AddObjectToSet(buffer);
  return buffer;
}

std::shared_ptr<BufferOpenGL> RenderContextOpenGL::CreateVertexBuffer(const void* data, size_t size,
                                                                      BufferUsage usage, BufferAccess access) {
  return CreateBuffer(data, size, BufferType::VertexBuffer, usage, access);
}

std::shared_ptr<BufferOpenGL> RenderContextOpenGL::CreateIndexBuffer(const void* data, size_t size,
                                                                     BufferUsage usage, BufferAccess access) {
  return CreateBuffer(data, size, BufferType::IndexBuffer, usage, access);
}

std::shared_ptr<BufferOpenGL> RenderContextOpenGL::CreateUniformBuffer(const void* data, size_t size,
                                                                       BufferUsage usage, BufferAccess access) {
  return CreateBuffer(data, size, BufferType::UniformBuffer, usage, access);
}

std::shared_ptr<ProgramOpenGL> RenderContextOpenGL::CreateShaderProgram(const std::string& vs, const std::string& fs,
                                                                        const ShaderAttributeLayouts& desc) {
  CheckInit();
  ShaderOpenGL vShader(ShaderType::Vertex, vs);
  if (!vShader.IsValid()) {
    throw RenderContextException("Compile vertex shader failed");
  }
  ShaderOpenGL fShader(ShaderType::Fragment, fs);
  if (!vShader.IsValid()) {
    throw RenderContextException("Compile fragment shader failed");
  }
  auto program = std::make_shared<ProgramOpenGL>(vShader, fShader, desc);
  vShader.Destroy();
  fShader.Destroy();
  if (!program->IsValid()) {
    throw RenderContextException("Link shader failed");
  }
  AddUniformBlocks(*program);
  AddObjectToSet(program);
  VertexArrayOpenGL vao(program->GetAttributes());
  auto result = _vaos.emplace(program, std::move(vao));
  if (!result.second) {
    throw RenderContextException("can't create VAO for program");
  }
  //assert(isInsert);
  return program;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::CreateTexture2D(const Texture2dDescriptorOpenGL& desc) {
  CheckInit();
  auto texture = std::make_shared<TextureOpenGL>(desc);
  AddObjectToSet(texture);
  return texture;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::CreateCubeMap(const TextureCubeMapDescriptorOpenGL& desc) {
  CheckInit();
  auto texture = std::make_shared<TextureOpenGL>(desc);
  AddObjectToSet(texture);
  return texture;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::CreateDepthTexture(const DepthTextureDescriptorOpenGL& desc) {
  CheckInit();
  auto texture = std::make_shared<TextureOpenGL>(desc);
  AddObjectToSet(texture);
  return texture;
}

std::shared_ptr<FrameBufferOpenGL> RenderContextOpenGL::CreateFrameBuffer(const FrameBufferDepthDescriptor& desc) {
  CheckInit();
  auto fbo = std::make_shared<FrameBufferOpenGL>(desc);
  AddObjectToSet(fbo);
  return fbo;
}

std::shared_ptr<FrameBufferOpenGL> RenderContextOpenGL::CreateFrameBuffer(const FrameBufferRenderDescriptor& desc) {
  CheckInit();
  auto fbo = std::make_shared<FrameBufferOpenGL>(desc);
  AddObjectToSet(fbo);
  return fbo;
}

std::shared_ptr<RenderBufferOpenGL> RenderContextOpenGL::CreateRenderBuffer(const RenderBufferDescriptor& desc) {
  CheckInit();
  auto rbo = std::make_shared<RenderBufferOpenGL>(desc);
  AddObjectToSet(rbo);
  return rbo;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::ConvertSphericalToCubemap(
    const Texture2dDescriptorOpenGL& tex2d,
    const TextureCubeMapDescriptorOpenGL& cubeConfig,
    const std::filesystem::path& shaderLib) {
  for (int i = 0; i < 6; i++) {
    if (cubeConfig.DataPtr[i] != nullptr) {
      throw OpenGLException("cube map DataPtr should be null");
    }
  }
  auto rect = CreateTexture2D(tex2d);        //上传纹理
  auto cubemap = CreateCubeMap(cubeConfig);  //为cubemap分配空间
  std::shared_ptr<BufferOpenGL> vbo;         //立方体
  int verCnt{};
  {
    auto cubeModel = ImmutableModel::CreateCube("cube", 1);
    auto d = GenVboDataPNT(cubeModel.GetPosition(), {}, {}, cubeModel.GetIndices());
    auto data = d.data();
    auto size = d.size() * sizeof(decltype(d)::value_type);
    vbo = CreateVertexBuffer(data, size);
    verCnt = int(cubeModel.GetIndexCount());
  }
  ImmutableText vs("vs", shaderLib / "SphericalToCubeMap.vert");
  ImmutableText fs("fs", shaderLib / "SphericalToCubeMap.frag");
  auto prog = CreateShaderProgram(vs.GetText(), fs.GetText(), {POSITION0()});  //转换shader
  GLuint captureFBO, captureRBO;
  HIKARI_CHECK_GL(glGenFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glGenRenderbuffers(1, &captureRBO));
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
  HIKARI_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
  //附加深度
  HIKARI_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeConfig.Width, cubeConfig.Height));
  HIKARI_CHECK_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO));
  Matrix4f proj = Perspective(Radian(90), 1.0f, 0.1f, 10.0f);
  Matrix4f view[6] = {
      LookAt(Vector3f{0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f})};
  HIKARI_CHECK_GL(glEnable(GL_DEPTH_TEST));
  prog->Bind();
  auto& vao = GetVertexArray(prog);
  vao.Bind();
  HIKARI_CHECK_GL(glActiveTexture(GL_TEXTURE0));
  HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_2D, rect->GetHandle()));
  prog->UniformMat4("projection", proj.GetAddress());
  prog->UniformTexture2D("u_equirectangularMap", 0);
  SetViewport(0, 0, cubeConfig.Width, cubeConfig.Height);
  for (int i = 0; i < 6; i++) {
    prog->UniformMat4("view", view[i].GetAddress());
    HIKARI_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                           GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                           cubemap->GetHandle(),
                                           0));
    HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    auto layout = GetVertexLayoutPositionPNT();
    auto bp = prog->GetBindingPoint(layout.Semantic);
    vao.SetVertexBuffer({(GLuint)bp, vbo->GetHandle(), layout.Offset, layout.Stride});
    DrawArrays(PrimitiveMode::Triangles, 0, verCnt);
  }
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  //清理
  DestroyObject(rect);
  DestroyObject(vbo);
  DestroyObject(prog);
  HIKARI_CHECK_GL(glDeleteFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glDeleteRenderbuffers(1, &captureRBO));

  return cubemap;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::GenIrradianceConvolutionCubemap(
    const std::shared_ptr<TextureOpenGL>& irradiance,
    const TextureCubeMapDescriptorOpenGL& cubeConfig,
    const std::filesystem::path& shaderLib) {
  auto cubemap = CreateCubeMap(cubeConfig);  //为cubemap分配空间
  std::shared_ptr<BufferOpenGL> vbo;         //立方体
  int verCnt{};
  {
    auto cubeModel = ImmutableModel::CreateCube("cube", 1);
    auto d = GenVboDataPNT(cubeModel.GetPosition(), {}, {}, cubeModel.GetIndices());
    auto data = d.data();
    auto size = d.size() * sizeof(decltype(d)::value_type);
    vbo = CreateVertexBuffer(data, size);
    verCnt = int(cubeModel.GetIndexCount());
  }
  ImmutableText vs("vs", shaderLib / "IrradianceConvolution.vert");
  ImmutableText fs("fs", shaderLib / "IrradianceConvolution.frag");
  auto prog = CreateShaderProgram(vs.GetText(), fs.GetText(), {POSITION0()});  //转换shader
  GLuint captureFBO, captureRBO;
  HIKARI_CHECK_GL(glGenFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glGenRenderbuffers(1, &captureRBO));
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
  HIKARI_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
  HIKARI_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeConfig.Width, cubeConfig.Height));
  HIKARI_CHECK_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO));
  Matrix4f proj = Perspective(Radian(90), 1.0f, 0.1f, 10.0f);
  Matrix4f view[6] = {
      LookAt(Vector3f{0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f})};
  HIKARI_CHECK_GL(glEnable(GL_DEPTH_TEST));
  prog->Bind();
  auto& vao = GetVertexArray(prog);
  vao.Bind();
  HIKARI_CHECK_GL(glActiveTexture(GL_TEXTURE0));
  HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance->GetHandle()));
  prog->UniformMat4("projection", proj.GetAddress());
  prog->UniformCubeMap("u_Cube", 0);
  SetViewport(0, 0, cubeConfig.Width, cubeConfig.Height);
  for (int i = 0; i < 6; i++) {
    prog->UniformMat4("view", view[i].GetAddress());
    HIKARI_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                           GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                           cubemap->GetHandle(),
                                           0));
    HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    auto layout = GetVertexLayoutPositionPNT();
    auto bp = prog->GetBindingPoint(layout.Semantic);
    vao.SetVertexBuffer({(GLuint)bp, vbo->GetHandle(), layout.Offset, layout.Stride});
    DrawArrays(PrimitiveMode::Triangles, 0, verCnt);
  }
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  //清理
  DestroyObject(vbo);
  DestroyObject(prog);
  HIKARI_CHECK_GL(glDeleteFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glDeleteRenderbuffers(1, &captureRBO));

  return cubemap;
}

std::shared_ptr<TextureOpenGL> RenderContextOpenGL::PrefilterEnvMap(
    const std::shared_ptr<TextureOpenGL>& env,
    const TextureCubeMapDescriptorOpenGL& config,
    const std::filesystem::path& shaderLib) {
  auto cfg = config;
  cfg.MipMapLevel = 0;
  cfg.MinFilter = FilterMode::Trilinear;
  cfg.MagFilter = FilterMode::Bilinear;
  for (int i = 0; i < 6; i++) {
    cfg.DataPtr[i] = nullptr;
  }
  auto cube = CreateCubeMap(cfg);
  ImmutableText vs("vs", shaderLib / "PreFilterEnv.vert");
  ImmutableText fs("fs", shaderLib / "PreFilterEnv.frag");
  auto prog = CreateShaderProgram(vs.GetText(), fs.GetText(), {POSITION0()});  //转换shader
  std::shared_ptr<BufferOpenGL> vbo;                                           //立方体
  int verCnt{};
  {
    auto cubeModel = ImmutableModel::CreateCube("cube", 1);
    auto d = GenVboDataPNT(cubeModel.GetPosition(), {}, {}, cubeModel.GetIndices());
    auto data = d.data();
    auto size = d.size() * sizeof(decltype(d)::value_type);
    vbo = CreateVertexBuffer(data, size);
    verCnt = int(cubeModel.GetIndexCount());
  }
  GLuint captureFBO, captureRBO;
  HIKARI_CHECK_GL(glGenFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glGenRenderbuffers(1, &captureRBO));
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, captureFBO));
  Matrix4f proj = Perspective(Radian(90), 1.0f, 0.1f, 10.0f);
  Matrix4f view[6] = {
      LookAt(Vector3f{0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}),
      LookAt(Vector3f{0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, -1.0f, 0.0f})};
  HIKARI_CHECK_GL(glEnable(GL_DEPTH_TEST));
  prog->Bind();
  auto& vao = GetVertexArray(prog);
  vao.Bind();
  HIKARI_CHECK_GL(glActiveTexture(GL_TEXTURE0));
  HIKARI_CHECK_GL(glBindTexture(GL_TEXTURE_CUBE_MAP, env->GetHandle()));
  prog->UniformMat4("projection", proj.GetAddress());
  prog->UniformCubeMap("u_Cube", 0);
  int maxMipLevels = TextureOpenGL::CalcMipmapLevels(cfg.MipMapLevel, cfg.Width, true);
  for (int mip = 0; mip < maxMipLevels; mip++) {
    int mipWidth = int(cfg.Width * std::pow(0.5f, float(mip)));
    int mipHeight = int(cfg.Height * std::pow(0.5, float(mip)));
    HIKARI_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, captureRBO));
    HIKARI_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight));
    HIKARI_CHECK_GL(glViewport(0, 0, mipWidth, mipHeight));
    float roughness = (float)mip / (float)(maxMipLevels - 1);
    prog->UniformFloat("u_roughness", roughness);
    for (uint32_t i = 0; i < 6; i++) {
      prog->UniformMat4("view", view[i].GetAddress());
      HIKARI_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,
                                             GL_COLOR_ATTACHMENT0,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                             cube->GetHandle(),
                                             mip));
      HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
      auto layout = GetVertexLayoutPositionPNT();
      auto bp = prog->GetBindingPoint(layout.Semantic);
      vao.SetVertexBuffer({(GLuint)bp, vbo->GetHandle(), layout.Offset, layout.Stride});
      DrawArrays(PrimitiveMode::Triangles, 0, verCnt);
    }
  }
  HIKARI_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  DestroyObject(vbo);
  DestroyObject(prog);
  HIKARI_CHECK_GL(glDeleteFramebuffers(1, &captureFBO));
  HIKARI_CHECK_GL(glDeleteRenderbuffers(1, &captureRBO));

  return cube;
}

void RenderContextOpenGL::AddUniformBlocks(const ProgramOpenGL& prog) {
  CheckInit();
  for (const auto& block : prog.GetBlocks()) {
    auto iter = _blockQueryMap.find(block.Name);
    GLuint bindingPoint;
    if (iter == _blockQueryMap.end()) {
      GlobalUniformBlock binding;
      binding.Block = block;
      binding.BindingPoint = std::distance(_globalBlocks.begin(), _globalBlocks.end());
      binding.Data.resize(block.DataSize, 0);
      binding.Ubo = CreateUniformBuffer(nullptr, block.DataSize, BufferUsage::Dynamic);
      _globalBlocks.emplace_back(binding);
      bindingPoint = GLuint(binding.BindingPoint);
      HIKARI_CHECK_GL(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, binding.Ubo->GetHandle()));
      _blockQueryMap.emplace(block.Name, binding.BindingPoint);
      for (const auto& member : block.Members) {
        auto result = _globalUniforms.emplace(member.Name, GlobalUniform{member, binding.BindingPoint});
        if (!result.second) {
          throw RenderContextException("uniform block member has same name");
        }
      }
    } else {
      const auto& global = _globalBlocks[iter->second];
      if (global.Block != block) {
        throw RenderContextException("Uniform block has same name but different layout");
      }
      bindingPoint = GLuint(global.BindingPoint);
    }
    HIKARI_CHECK_GL(glUniformBlockBinding(prog.GetHandle(), block.Index, bindingPoint));
  }
}

void RenderContextOpenGL::DestroyObject(const std::shared_ptr<ObjectOpenGL>& ptr) {
  ptr->Destroy();
  auto count = _objects.erase(ptr);
  if (count == 0) {
    throw RenderContextException("This object is not created from this context");
  }
}

void RenderContextOpenGL::DestroyObject(const std::shared_ptr<ProgramOpenGL>& ptr) {
  auto count = _vaos.erase(ptr);
  auto cast = std::static_pointer_cast<ObjectOpenGL, ProgramOpenGL>(ptr);
  DestroyObject(cast);
  if (count == 0) {
    throw RenderContextException("This shader program has't VAO");
  }
}

std::optional<const VertexArrayOpenGL*> RenderContextOpenGL::TryGetVertexArray(const std::shared_ptr<ProgramOpenGL>& ptr) const {
  auto iter = _vaos.find(ptr);
  return iter == _vaos.end() ? std::nullopt : std::make_optional(&iter->second);
}

const VertexArrayOpenGL& RenderContextOpenGL::GetVertexArray(const std::shared_ptr<ProgramOpenGL>& ptr) const { return _vaos.at(ptr); }

static EShLanguage MapShaderTypToGlslang(ShaderType type) {
  switch (type) {
    case Hikari::ShaderType::Fragment:
      return EShLanguage::EShLangFragment;
    case Hikari::ShaderType::Vertex:
      return EShLanguage::EShLangVertex;
    default:
      throw RenderContextException("unknown shader type");
  }
}

static void __OutputStrLog(const char* str) {
  if (str && str[0]) {
    std::cout << str << std::endl;
  }
}

bool RenderContextOpenGL::PreprocessShader(ShaderType type, const std::string& source, std::string& res) {
  //CheckInit();
  EShLanguage lang = MapShaderTypToGlslang(type);
  glslang::InitializeProcess();
  auto shader = std::make_unique<glslang::TShader>(lang);
  shader->setEnvInput(glslang::EShSourceGlsl, EShLanguage::EShLangVertex, glslang::EShClient::EShClientOpenGL, 100);
  shader->setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EshTargetClientVersion::EShTargetOpenGL_450);
  shader->setEnvTarget(glslang::EShTargetLanguage::EShTargetNone, (glslang::EShTargetLanguageVersion)0);
  shader->setAutoMapBindings(true);
  shader->setAutoMapLocations(true);
  auto src = source.c_str();
  shader->setStrings(&src, 1);
  shader->setPreamble("\n#extension GL_GOOGLE_include_directive : enable\n");  //启用#include
  std::string result;
  bool proc = shader->preprocess(&__BuiltInRes, 110, ECoreProfile, false, false, EShMsgDefault, &result, _includer);
  if (proc) {
    proc = shader->parse(&__BuiltInRes, 110, false, EShMsgDefault, _includer);
    if (!proc) {
      std::cout << "--------------parse info---------------" << std::endl;
      __OutputStrLog(shader->getInfoLog());
      __OutputStrLog(shader->getInfoDebugLog());
      std::cout << "---------------------------------------" << std::endl;
    }
  } else {
    std::cout << "------------preprocess info------------" << std::endl;
    __OutputStrLog(shader->getInfoLog());
    __OutputStrLog(shader->getInfoDebugLog());
    std::cout << "---------------------------------------" << std::endl;
  }
  if (proc) {
    const std::string verCmd("#version");
    const std::string lineCmd("#line");
    const std::string incCmd("#extension GL_GOOGLE_include_directive");
    std::stringstream i(result);
    res = std::string();
    bool canAppend = false;
    while (!i.eof()) {
      std::string line;
      std::getline(i, line);
      if (line.compare(0, verCmd.size(), verCmd) == 0) {  //StartWith
        canAppend = true;
      }
      if (line.compare(0, lineCmd.size(), lineCmd) == 0) {  //忽略掉include时插入的预处理语句
        continue;
      }
      if (line.compare(0, incCmd.size(), incCmd) == 0) {
        continue;
      }
      if (canAppend) {
        res.append(line);
        res.append("\n");
      }
    }
  }
  glslang::InitializeProcess();
  return proc;
}

bool RenderContextOpenGL::ProcessShader(ShaderType type, const std::string& source, std::string& res) {
  CheckInit();
  EShLanguage lang = MapShaderTypToGlslang(type);
  glslang::InitializeProcess();
  auto shader = std::make_unique<glslang::TShader>(lang);
  //最后一个参数version不知道啥玩意，看StandAlone.cpp只要是opengl和vulkan就填100
  shader->setEnvInput(glslang::EShSourceGlsl, EShLanguage::EShLangVertex, glslang::EShClient::EShClientOpenGL, 100);
  shader->setEnvClient(glslang::EShClient::EShClientOpenGL, glslang::EshTargetClientVersion::EShTargetOpenGL_450);
  shader->setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);
  shader->setAutoMapBindings(true);
  shader->setAutoMapLocations(true);
  auto src = source.c_str();
  shader->setStrings(&src, 1);
  shader->setPreamble("\n#extension GL_GOOGLE_include_directive : enable\n");
  //defaultVersion，桌面填110，ES填100（没看出有啥区别
  bool parseResult = shader->parse(&__BuiltInRes, 110, false, EShMsgDefault, _includer);
  if (!parseResult) {
    res = std::string(shader->getInfoLog());
    glslang::FinalizeProcess();
    return false;
  }
  auto program = std::make_unique<glslang::TProgram>();
  program->addShader(shader.get());
  bool linkResult = program->link(EShMsgDefault);
  if (!linkResult) {
    res = std::string(program->getInfoLog());
    glslang::FinalizeProcess();
    return false;
  }
  glslang::SpvOptions spvOpt;
  spvOpt.validate = true;
  spvOpt.disableOptimizer = false;
  spvOpt.optimizeSize = true;
  std::vector<uint32_t> spirv;
  spv::SpvBuildLogger logger;
  glslang::GlslangToSpv(*program->getIntermediate(lang), spirv, &logger, &spvOpt);
  if (spirv.empty()) {
    res = logger.getAllMessages();
    glslang::FinalizeProcess();
    return false;
  }
  auto compiler = std::make_unique<spirv_cross::CompilerGLSL>(spirv);
  auto ress = compiler->get_shader_resources();
  auto cmpOpts = compiler->get_common_options();
  const auto& feature = FeatureOpenGL::Get();
  cmpOpts.enable_420pack_extension = false;
  cmpOpts.es = false;
  cmpOpts.version = feature.GetMaxGlslVersion();
  cmpOpts.flatten_multidimensional_arrays = true;
  compiler->set_common_options(cmpOpts);
  auto result = compiler->compile();
  res = result;
  glslang::FinalizeProcess();
  return true;
}

RenderContextOpenGL::~RenderContextOpenGL() noexcept {
  Destroy();
}

void RenderContextOpenGL::Init(const std::filesystem::path& shaderPath) {
  _includer.AddSystemPath(shaderPath);
  _isValid = true;
}

bool RenderContextOpenGL::IsValid() const {
  return _isValid;
}

void RenderContextOpenGL::SetClearColor(float r, float g, float b, float a) const {
  HIKARI_CHECK_GL(glClearColor(r, g, b, a));
}

void RenderContextOpenGL::ClearColor() const {
  HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
}

void RenderContextOpenGL::ClearDepth(float depth) const {
  HIKARI_CHECK_GL(glClearDepth(depth));
}

void RenderContextOpenGL::ClearDepth() const {
  HIKARI_CHECK_GL(glClear(GL_DEPTH_BUFFER_BIT));
}

void RenderContextOpenGL::ClearColorAndDepth() const {
  HIKARI_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void RenderContextOpenGL::SetViewport(int x, int y, int width, int height) const {
  HIKARI_CHECK_GL(glViewport(x, y, width, height));
}

void RenderContextOpenGL::ColorMask(bool r, bool g, bool b, bool a) const {
  HIKARI_CHECK_GL(glColorMask(r, g, b, a));
}

void RenderContextOpenGL::DrawArrays(PrimitiveMode mode, int first, int count) const {
  HIKARI_CHECK_GL(glDrawArrays(MapPrimitiveMode(mode), first, count));
}

void RenderContextOpenGL::DrawElements(PrimitiveMode mode, int count, IndexDataType type, size_t first) const {
  HIKARI_CHECK_GL(glDrawElements(MapPrimitiveMode(mode), count, MapIndexDataType(type), (void*)first));
}

void RenderContextOpenGL::SubmitGlobalUnifroms() const {
  for (const auto& block : _globalBlocks) {
    block.Ubo->UpdateData(0, block.Block.DataSize, block.Data.data());
  }
}

void RenderContextOpenGL::SetGlobalUniformData(const std::string& name,
                                               size_t dataSize,
                                               int length,
                                               int align,
                                               const void* data) {
  auto iter = _globalUniforms.find(name);
  if (iter == _globalUniforms.end()) {
    return;
  }
  const auto& uniform = iter->second;
  SetGlobalUniformData(uniform, dataSize, length, align, data);
}

void RenderContextOpenGL::SetGlobalUniformData(const GlobalUniform& uniform,
                                               size_t dataSize,
                                               int length,
                                               int align,
                                               const void* data) {
  if (ProgramOpenGL::MapParamSize(uniform.Info.Type) != dataSize) {
    throw RenderContextException("invalid data size");
  }
  if (length < 0 || length > uniform.Info.Length) {
    throw RenderContextException("out of range");
  }
  if (length > 1 && align != uniform.Info.Align) {
    throw RenderContextException("invalid align");
  }
  if (length == 0) {
    return;
  }
  auto allSize = length > 1 ? size_t(align) * size_t(length) : dataSize;
  auto& block = _globalBlocks[uniform.BlockHandle];
  auto head = reinterpret_cast<const std::uint8_t*>(data);
  auto target = block.Data.begin() + uniform.Info.Offset;
  std::copy(head, head + allSize, target);
}

void RenderContextOpenGL::SetGlobalUniform(const std::string& name,
                                           size_t dataSize,
                                           int length,
                                           int align,
                                           const void* data) {
  auto iter = _globalUniforms.find(name);
  if (iter == _globalUniforms.end()) {
    return;
  }
  const auto& uniform = iter->second;
  SetGlobalUniformData(uniform, dataSize, length, align, data);
  //auto& block = _globalBlocks[uniform.BlockHandle];
  //block.Ubo->UpdateData(uniform.Info.Offset, int(dataSize * length), data);
}

void RenderContextOpenGL::SetGlobalFloat(const std::string& name, float value) { SetGlobalUniform(name, sizeof(float), 1, 0, &value); }
void RenderContextOpenGL::SetGlobalInt(const std::string& name, int value) { SetGlobalUniform(name, sizeof(int), 1, 0, &value); }
void RenderContextOpenGL::SetGlobalMat4(const std::string& name, const Matrix4f& value) { SetGlobalUniform(name, sizeof(Matrix4f), 1, 0, value.GetAddress()); }
void RenderContextOpenGL::SetGlobalVec3(const std::string& name, const Vector3f& value) { SetGlobalUniform(name, sizeof(Vector3f), 1, 0, value.GetAddress()); }
void RenderContextOpenGL::SetGlobalTex2d(const std::string& name, const TextureOpenGL& tex2d) {
  auto handle = tex2d.GetHandle();
  SetGlobalUniform(name, sizeof(decltype(handle)), 1, 0, &handle);
}
void RenderContextOpenGL::SetGlobalCubeMap(const std::string& name, const TextureOpenGL& cubemap) {
  auto handle = cubemap.GetHandle();
  SetGlobalUniform(name, sizeof(decltype(handle)), 1, 0, &handle);
}
void RenderContextOpenGL::SetGlobalFloatArray(const std::string& name, const void* value, int length) { SetGlobalUniform(name, sizeof(float), length, 4, value); }
void RenderContextOpenGL::SetGlobalIntArray(const std::string& name, const void* value, int length) { SetGlobalUniform(name, sizeof(int), length, 4, value); }
void RenderContextOpenGL::SetGlobalMat4Array(const std::string& name, const void* value, int length) { SetGlobalUniform(name, sizeof(Matrix4f), length, 64, value); }
void RenderContextOpenGL::SetGlobalVec3Array(const std::string& name, const void* value, int length) { SetGlobalUniform(name, sizeof(Vector3f), length, 16, value); }
void RenderContextOpenGL::SetGlobalTex2dArray(const std::string& name, const void* tex2d, int length) { SetGlobalUniform(name, sizeof(GLuint), length, 4, tex2d); }
void RenderContextOpenGL::SetGlobalCubeMapArray(const std::string& name, const void* cubemap, int length) { SetGlobalUniform(name, sizeof(GLuint), length, 4, cubemap); }

void RenderContextOpenGL::CheckInit() const {
#if !defined(NDEBUG)
  if (!IsValid()) {
    throw RenderContextException("OpenGL context is not initialized");
  }
#endif  // define
}

void RenderContextOpenGL::AddObjectToSet(const std::shared_ptr<ObjectOpenGL>& obj) {
  auto result = _objects.emplace(obj->shared_from_this());
  if (!result.second) {
    throw RenderContextException("object has been added");
  }
}

std::vector<VertexPositionNormalTexCoord> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                                        const std::vector<Vector3f>& normal,
                                                        const std::vector<Vector2f>& tex) {
  std::vector<VertexPositionNormalTexCoord> pnt(pos.size(), VertexPositionNormalTexCoord{});
  for (size_t i = 0; i < pos.size(); i++) {
    auto p = pos[i];
    auto n = normal.size() > 0 ? normal[i] : Vector3f(0.0f);
    auto t = tex.size() > 0 ? tex[i] : Vector2f(0.0f);
    pnt[i] = {p, n, t};
  }
  return pnt;
}

std::vector<VertexPositionNormalTexCoord> GenVboDataPNT(const std::vector<Vector3f>& pos,
                                                        const std::vector<Vector3f>& normal,
                                                        const std::vector<Vector2f>& tex,
                                                        const std::vector<size_t>& idx) {
  std::vector<VertexPositionNormalTexCoord> pnt(idx.size(), VertexPositionNormalTexCoord{});
  for (size_t i = 0; i < idx.size(); i++) {
    auto p = pos[idx[i]];
    auto n = normal.size() > 0 ? normal[idx[i]] : Vector3f(0.0f);
    auto t = tex.size() > 0 ? tex[idx[i]] : Vector2f(0.0f);
    pnt[i] = {p, n, t};
  }
  return pnt;
}

ShaderAttributeLayout POSITION0() {
  return ShaderAttributeLayout{"a_Pos", SemanticType::Vertex, 0};
}

ShaderAttributeLayout NORMAL0() {
  return ShaderAttributeLayout{"a_Normal", SemanticType::Normal, 0};
}

}  // namespace Hikari