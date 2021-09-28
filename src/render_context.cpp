#include <hikari/render_context.h>

#include <iostream>
#include <cassert>
#include <algorithm>
#include <stdexcept>

namespace Hikari {

RenderContextOpenGL::RenderContextOpenGL() noexcept = default;

RenderContextOpenGL::RenderContextOpenGL(RenderContextOpenGL&& other) noexcept {
  _objects = std::move(other._objects);
  _vaos = std::move(other._vaos);
  _isValid = other._isValid;
  other._isValid = false;
}

RenderContextOpenGL& RenderContextOpenGL::operator=(RenderContextOpenGL&& other) noexcept {
  _objects = std::move(other._objects);
  _vaos = std::move(other._vaos);
  _isValid = other._isValid;
  other._isValid = false;
  return *this;
}

void RenderContextOpenGL::Destroy() {
  for (auto& [_, vao] : _vaos) {
    vao.Destroy();
  }
  for (const auto& obj : _objects) {
    obj->Destroy();
  }
}

std::shared_ptr<BufferOpenGL> RenderContextOpenGL::CreateEmptyBuffer(BufferType type,
                                                                     BufferUsage usage,
                                                                     BufferAccess access) {
  CheckInit();
  auto buffer = std::make_shared<BufferOpenGL>(type, usage, access);
  AddObjectToSet(buffer);
  return buffer;
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
  AddObjectToSet(program);

  VertexArrayOpenGL vao(program->GetAttributes());
  auto [_, isInsert] = _vaos.emplace(program, std::move(vao));
  assert(isInsert);

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

const VertexArrayOpenGL& RenderContextOpenGL::GetVertexArray(const std::shared_ptr<ProgramOpenGL>& ptr) const {
  return _vaos.at(ptr);
}

RenderContextOpenGL::~RenderContextOpenGL() noexcept {
  Destroy();
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

void RenderContextOpenGL::CheckInit() const {
#if !defined(NDEBUG)
  if (!IsValid()) {
    throw RenderContextException("OpenGL context is not initialized");
  }
#endif  // define
}

void RenderContextOpenGL::AddObjectToSet(const std::shared_ptr<ObjectOpenGL>& obj) {
  auto [_, isInsert] = _objects.emplace(obj->shared_from_this());
  assert(isInsert);
}

void CreateVboAndIboFromModelPNT(const ImmutableModel& model, BufferOpenGL& vbo, BufferOpenGL& ibo) {
  std::vector<VertexPositionNormalTexCoord> pnt;
  std::vector<uint32_t> idx;
  pnt.reserve(model.GetVertexCount());
  idx.reserve(model.GetIndexCount());
  for (size_t i = 0; i < model.GetVertexCount(); i++) {
    auto p = model.GetPosition()[i];
    auto n = model.HasNormal() ? model.GetNormals()[i] : Vector3f(0.0f);
    auto t = model.HasTexCoord() ? model.GetTexCoords()[i] : Vector2f(0.0f);
    pnt.emplace_back(VertexPositionNormalTexCoord{p, n, t});
  }
  for (size_t i = 0; i < model.GetIndexCount(); i++) {
    idx.emplace_back(static_cast<uint32_t>(model.GetIndices()[i]));
  }
  vbo.Store(pnt.data(), pnt.size() * sizeof(decltype(pnt)::value_type));
  ibo.Store(idx.data(), idx.size() * sizeof(decltype(idx)::value_type));
}

void CreateVboFormModelPNT(const ImmutableModel& model, BufferOpenGL& vbo) {
  std::vector<VertexPositionNormalTexCoord> pnt(model.GetIndexCount(), VertexPositionNormalTexCoord{});
  for (size_t i = 0; i < model.GetIndexCount(); i++) {
    auto p = model.GetPosition()[model.GetIndices()[i]];
    auto n = model.HasNormal() ? model.GetNormals()[model.GetIndices()[i]] : Vector3f(0.0f);
    auto t = model.HasTexCoord() ? model.GetTexCoords()[model.GetIndices()[i]] : Vector2f(0.0f);
    pnt[i] = {p, n, t};
  }
  vbo.Store(pnt.data(), pnt.size() * sizeof(decltype(pnt)::value_type));
}

}  // namespace Hikari