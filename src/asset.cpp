#include <hikari/asset.h>

#include <iostream>
#include <map>
#include <fstream>

#include <stb_image.h>
#include <tiny_obj_loader.h>

namespace Hikari {
Asset::Asset() noexcept = default;

Asset::~Asset() noexcept = default;

ImmutableBitmap::ImmutableBitmap() noexcept = default;

ImmutableBitmap::~ImmutableBitmap() noexcept {
  if (_data != nullptr) {
    stbi_image_free(_data);
    _data = nullptr;
  }
}

ImmutableBitmap::ImmutableBitmap(const std::string& name, const std::filesystem::path& path, bool isFilpY) {
  if (!LoadFromDisk(name, path, isFilpY, *this)) {
    throw AssetLoadException("can't load from disk");
  }
}

ImmutableBitmap::ImmutableBitmap(ImmutableBitmap&& other) noexcept {
  _width = other._width;
  _height = other._height;
  _channelCount = other._channelCount;
  _data = other._data;
  other._data = nullptr;
  _name = std::move(other._name);
}

ImmutableBitmap& ImmutableBitmap::operator=(ImmutableBitmap&& other) noexcept {
  _width = other._width;
  _height = other._height;
  _channelCount = other._channelCount;
  _data = other._data;
  other._data = nullptr;
  _name = std::move(other._name);
  return *this;
}

const std::string& ImmutableBitmap::GetName() const {
  return _name;
}

bool ImmutableBitmap::IsValid() const {
  return _data != nullptr;
}

void ImmutableBitmap::Release() {
  if (_data != nullptr) {
    stbi_image_free(_data);
    _data = nullptr;
  }
}

int ImmutableBitmap::GetWidth() const {
  return _width;
}

int ImmutableBitmap::GetHeight() const {
  return _height;
}

int ImmutableBitmap::GetChannel() const {
  return _channelCount;
}

const uint8_t* const ImmutableBitmap::GetData() const {
  return _data;
}

bool ImmutableBitmap::LoadFromDisk(const std::string& name,
                                   const std::filesystem::path& p,
                                   bool filpY,
                                   ImmutableBitmap& texture) {
  stbi_set_flip_vertically_on_load(filpY);
  int width, height, channels;
  auto data = stbi_load((const char*)p.generic_u8string().c_str(), &width, &height, &channels, 0);
  texture._width = width;
  texture._height = height;
  texture._channelCount = channels;
  texture._data = data;
  texture._name = name;
  return data != nullptr;
}

ImmutableHdrTexture::ImmutableHdrTexture() noexcept = default;

ImmutableHdrTexture::ImmutableHdrTexture(const std::string& name, const std::filesystem::path& path, bool isFilpY) {
  stbi_set_flip_vertically_on_load(isFilpY);
  int width, height, channels;
  auto data = stbi_loadf((const char*)path.generic_u8string().c_str(), &width, &height, &channels, 0);
  if (data == nullptr) {
    throw AssetLoadException("can't load from disk");
  }
  _width = width;
  _height = height;
  _channelCount = channels;
  _data = data;
  _name = name;
}

ImmutableHdrTexture::ImmutableHdrTexture(ImmutableHdrTexture&& other) noexcept {
  _width = other._width;
  _height = other._height;
  _channelCount = other._channelCount;
  _data = other._data;
  other._data = nullptr;
  _name = std::move(other._name);
}

ImmutableHdrTexture& ImmutableHdrTexture::operator=(ImmutableHdrTexture&& other) noexcept {
  _width = other._width;
  _height = other._height;
  _channelCount = other._channelCount;
  _data = other._data;
  other._data = nullptr;
  _name = std::move(other._name);
  return *this;
}

ImmutableHdrTexture::~ImmutableHdrTexture() noexcept {
  if (_data != nullptr) {
    stbi_image_free(_data);
    _data = nullptr;
  }
}

const std::string& ImmutableHdrTexture::GetName() const { return _name; }

bool ImmutableHdrTexture::IsValid() const { return _data != nullptr; }

void ImmutableHdrTexture::Release() {
  if (_data != nullptr) {
    stbi_image_free(_data);
    _data = nullptr;
  }
}

int ImmutableHdrTexture::GetWidth() const { return _width; }

int ImmutableHdrTexture::GetHeight() const { return _height; }

int ImmutableHdrTexture::GetChannel() const { return _channelCount; }

const float* const ImmutableHdrTexture::GetData() const { return _data; }

ImmutableModel::ImmutableModel() noexcept = default;

ImmutableModel::ImmutableModel(const std::string& name, const std::filesystem::path& path) {
  if (!LoadFromFile(name, path, *this)) {
    throw AssetLoadException("can't load from disk");
  }
}

ImmutableModel::ImmutableModel(const std::string& name,
                               std::vector<Vector3f>&& pos,
                               std::vector<Vector3f>&& nor,
                               std::vector<Vector2f>&& tex,
                               std::vector<size_t>&& ind) noexcept {
  _name = name;
  _positions = std::move(pos);
  _normals = std::move(nor);
  _texcoords = std::move(tex);
  _indices = std::move(ind);
}

ImmutableModel::ImmutableModel(const std::string& name,
                               std::vector<Vector3f>&& pos,
                               std::vector<Vector3f>&& nor,
                               std::vector<Vector2f>&& tex,
                               std::vector<Vector4f>&& tan,
                               std::vector<size_t>&& ind) noexcept {
  _name = name;
  _positions = std::move(pos);
  _normals = std::move(nor);
  _texcoords = std::move(tex);
  _tangent = std::move(tan);
  _indices = std::move(ind);
}

ImmutableModel::ImmutableModel(ImmutableModel&& other) noexcept {
  _positions = std::move(other._positions);
  _normals = std::move(other._normals);
  _texcoords = std::move(other._texcoords);
  _tangent = std::move(other._tangent);
  _indices = std::move(other._indices);
  _name = std::move(other._name);
}

ImmutableModel& ImmutableModel::operator=(ImmutableModel&& other) noexcept {
  _positions = std::move(other._positions);
  _normals = std::move(other._normals);
  _texcoords = std::move(other._texcoords);
  _tangent = std::move(other._tangent);
  _indices = std::move(other._indices);
  _name = std::move(other._name);
  return *this;
}

ImmutableModel::~ImmutableModel() noexcept {
  _positions.clear();
  _normals.clear();
  _texcoords.clear();
  _tangent.clear();
  _indices.clear();
  _positions.shrink_to_fit();
  _normals.shrink_to_fit();
  _texcoords.shrink_to_fit();
  _tangent.shrink_to_fit();
  _indices.shrink_to_fit();
}

const std::string& ImmutableModel::GetName() const {
  return _name;
}

bool ImmutableModel::IsValid() const {
  return _positions.size() > 0;
}

void ImmutableModel::Release() {
  _positions.clear();
  _normals.clear();
  _texcoords.clear();
  _tangent.clear();
  _indices.clear();
  _positions.shrink_to_fit();
  _normals.shrink_to_fit();
  _texcoords.shrink_to_fit();
  _tangent.shrink_to_fit();
  _indices.shrink_to_fit();
}

const std::vector<Vector3f>& ImmutableModel::GetPosition() const {
  return _positions;
}

const std::vector<Vector3f>& ImmutableModel::GetNormals() const {
  return _normals;
}

const std::vector<Vector2f>& ImmutableModel::GetTexCoords() const {
  return _texcoords;
}

const std::vector<Vector4f>& ImmutableModel::GetTangents() const {
  return _tangent;
}

const std::vector<size_t>& ImmutableModel::GetIndices() const {
  return _indices;
}

bool ImmutableModel::HasNormal() const {
  return _normals.size() > 0;
}

bool ImmutableModel::HasTexCoord() const {
  return _texcoords.size() > 0;
}

bool ImmutableModel::HasTangent() const {
  return _tangent.size() > 0;
}

size_t ImmutableModel::GetVertexCount() const {
  return _positions.size();
}

size_t ImmutableModel::GetIndexCount() const {
  return _indices.size();
}

size_t ImmutableModel::GetTriangleCount() const {
  assert(_indices.size() % 3 == 0);
  return _indices.size() / 3;
}

struct __VertexIdx {
  int p;
  int n;
  int t;
  bool operator<(const __VertexIdx& v) const {
    if (p < v.p) return true;
    if (p > v.p) return false;
    if (n < v.n) return true;
    if (n > v.n) return false;
    if (t < v.t) return true;
    if (t > v.t) return false;
    return false;
  }
};

bool ImmutableModel::LoadFromFile(const std::string& name, const std::filesystem::path& path, ImmutableModel& mesh) {
  using namespace tinyobj;
  ObjReader reader;
  bool isLoaded = reader.ParseFromFile((const char*)path.generic_u8string().c_str());
  if (!isLoaded || !reader.Valid()) {
    std::cout << ".obj parse error: " << reader.Error() << "\n";
    std::cout << ".obj parse warning: " << reader.Warning() << "\n";
    return false;
  }
  const auto& attribs = reader.GetAttrib();
  const auto& shapes = reader.GetShapes();
  size_t count = 0;
  std::map<__VertexIdx, size_t> uni;
  for (const auto& shape : shapes) {
    const auto& m = shape.mesh;
    for (size_t j = 0; j < m.indices.size(); j++) {
      const auto& idx = m.indices[j];
      __VertexIdx vid{idx.vertex_index, idx.normal_index, idx.texcoord_index};
      auto [iter, isIn] = uni.try_emplace(vid, count);
      if (isIn) {
        const auto& i = iter->first;
        Vector3f pos = {attribs.vertices[3 * (size_t)i.p + 0],
                        attribs.vertices[3 * (size_t)i.p + 1],
                        attribs.vertices[3 * (size_t)i.p + 2]};
        mesh._positions.emplace_back(pos);
        if (i.n >= 0) {
          Vector3f nor = {attribs.normals[3 * (size_t)i.n + 0],
                          attribs.normals[3 * (size_t)i.n + 1],
                          attribs.normals[3 * (size_t)i.n + 2]};
          mesh._normals.emplace_back(nor);
        }
        if (i.t >= 0) {
          Vector2f tex = {attribs.texcoords[2 * (size_t)i.t + 0],
                          attribs.texcoords[2 * (size_t)i.t + 1]};
          mesh._texcoords.emplace_back(tex);
        }
        count++;
      }
      mesh._indices.emplace_back(iter->second);
    }
  }
  mesh._positions.shrink_to_fit();
  mesh._normals.shrink_to_fit();
  mesh._texcoords.shrink_to_fit();

  if (mesh.HasNormal() && mesh.HasTexCoord()) {
    //http://foundationsofgameenginedev.com/FGED2-sample.pdf
    //第9页
    std::vector<Vector3f> tangent(mesh.GetVertexCount(), Vector3f{});
    std::vector<Vector3f> biTan(mesh.GetVertexCount(), Vector3f{});
    mesh._tangent.resize(mesh.GetVertexCount());
    //计算每个三角形的切线和副切线，叠加到三个顶点上
    for (size_t i = 0; i < mesh.GetIndexCount(); i += 3) {
      auto i0 = i + 0;
      auto i1 = i + 1;
      auto i2 = i + 2;
      auto p0 = mesh.GetPosition()[mesh._indices[i0]];
      auto p1 = mesh.GetPosition()[mesh._indices[i1]];
      auto p2 = mesh.GetPosition()[mesh._indices[i2]];
      auto w0 = mesh.GetTexCoords()[mesh._indices[i0]];
      auto w1 = mesh.GetTexCoords()[mesh._indices[i1]];
      auto w2 = mesh.GetTexCoords()[mesh._indices[i2]];

      auto e1 = p1 - p0;
      auto e2 = p2 - p0;
      auto x1 = w1.X() - w0.X();
      auto x2 = w2.X() - w0.X();
      auto y1 = w1.Y() - w0.Y();
      auto y2 = w2.Y() - w0.Y();

      float r = 1.0f / (x1 * y2 - x2 * y1);
      auto t = (e1 * Vector3f(y2) - e2 * Vector3f(y1)) * Vector3f(r);
      auto b = (e2 * Vector3f(x1) - e1 * Vector3f(x2)) * Vector3f(r);

      tangent[i0] += t;
      tangent[i1] += t;
      tangent[i2] += t;
      biTan[i0] += b;
      biTan[i1] += b;
      biTan[i2] += b;
    }
    //正交化所有切线并计算手性
    for (size_t i = 0; i < mesh.GetVertexCount(); i++) {
      const auto& t = tangent[i];
      const auto& b = biTan[i];
      const auto& n = mesh.GetNormals()[i];
      //应该是叫Gram-Schmidt process？
      auto xyz = Normalize(Reject(t, n));
      auto w = (Dot(Cross(t, b), n) > 0.0f) ? 1.0f : -1.0f;
      mesh._tangent[i] = {xyz.X(), xyz.Y(), xyz.Z(), w};
    }
  }
  mesh._name = name;
  return true;
}

ImmutableModel ImmutableModel::CreateSphere(const std::string& name, float radius, int numberSlices) {
  assert(numberSlices >= 3);

  const Vector3f axisX = {1.0f, 0.0f, 0.0f};

  uint32_t numberParallels = numberSlices / 2;
  uint32_t numberVertices = (numberParallels + 1) * (numberSlices + 1);
  uint32_t numberIndices = numberParallels * numberSlices * 6;

  float angleStep = (2.0f * PI) / ((float)numberSlices);

  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});

  for (uint32_t i = 0; i < numberParallels + 1; i++) {
    for (uint32_t j = 0; j < (uint32_t)(numberSlices + 1); j++) {
      uint32_t vertexIndex = (i * (numberSlices + 1) + j);
      uint32_t normalIndex = (i * (numberSlices + 1) + j);
      uint32_t texCoordsIndex = (i * (numberSlices + 1) + j);
      uint32_t tangentIndex = (i * (numberSlices + 1) + j);

      float px = radius * std::sin(angleStep * (float)i) * std::sin(angleStep * (float)j);
      float py = radius * std::cos(angleStep * (float)i);
      float pz = radius * std::sin(angleStep * (float)i) * std::cos(angleStep * (float)j);
      vertices[vertexIndex] = {px, py, pz};

      float nx = vertices[vertexIndex].X() / radius;
      float ny = vertices[vertexIndex].Y() / radius;
      float nz = vertices[vertexIndex].Z() / radius;
      normals[normalIndex] = {nx, ny, nz};

      float tx = (float)j / (float)numberSlices;
      float ty = 1.0f - (float)i / (float)numberParallels;
      texCoords[texCoordsIndex] = {tx, ty};

      Quaternionf quat({0, 1, 0}, 360.0f * texCoords[texCoordsIndex].X());
      auto mat = Rotate(quat);
      auto t = Matrix3f(mat) * axisX;
      tangents[tangentIndex] = {t.X(), t.Y(), t.Z(), 1.0f};
    }
  }

  uint32_t indexIndices = 0;
  std::vector<size_t> indices(numberIndices, size_t{});
  for (uint32_t i = 0; i < numberParallels; i++) {
    for (uint32_t j = 0; j < (uint32_t)(numberSlices); j++) {
      indices[indexIndices++] = i * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + ((size_t)j + 1);

      indices[indexIndices++] = i * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + ((size_t)j + 1);
      indices[indexIndices++] = (size_t)i * ((size_t)numberSlices + 1) + ((size_t)j + 1);
    }
  }

  return ImmutableModel(name,
                        std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateCube(const std::string& name, float halfExtend) {
  constexpr const float cubeVertices[] =
      {-1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, +1.0f,
       -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f,
       -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f, -1.0f, +1.0f,
       -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f,
       -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f,
       +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f};
  constexpr const float cubeNormals[] =
      {0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f,
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f,
       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f};
  constexpr const float cubeTexCoords[] =
      {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
  constexpr const float cubeTangents[] =
      {+1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f,
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f};
  constexpr const size_t cubeIndices[] =
      {0, 2, 1, 0, 3, 2,
       4, 5, 6, 4, 6, 7,
       8, 9, 10, 8, 10, 11,
       12, 15, 14, 12, 14, 13,
       16, 17, 18, 16, 18, 19,
       20, 23, 22, 20, 22, 21};

  constexpr const uint32_t numberVertices = 24;
  constexpr const uint32_t numberIndices = 36;

  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});
  for (uint32_t i = 0; i < numberVertices; i++) {
    vertices[i] = {cubeVertices[i * 4 + 0] * halfExtend,
                   cubeVertices[i * 4 + 1] * halfExtend,
                   cubeVertices[i * 4 + 2] * halfExtend};
    normals[i] = {cubeNormals[i * 3 + 0],
                  cubeNormals[i * 3 + 1],
                  cubeNormals[i * 3 + 2]};
    texCoords[i] = {cubeTexCoords[i * 2 + 0],
                    cubeTexCoords[i * 2 + 1]};
    tangents[i] = {cubeTangents[i * 2 + 0],
                   cubeTangents[i * 2 + 1],
                   cubeTangents[i * 2 + 2],
                   1.0f};
  }
  std::vector<size_t> indices(cubeIndices, cubeIndices + numberIndices);
  return ImmutableModel(name,
                        std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateQuad(const std::string& name, float halfExtend, float offset) {
  constexpr const float quadVertices[] =
      {-1.0f, -1.0f, 0.0f, +1.0f,
       +1.0f, -1.0f, 0.0f, +1.0f,
       -1.0f, +1.0f, 0.0f, +1.0f,
       +1.0f, +1.0f, 0.0f, +1.0f};
  constexpr const float quadNormal[] =
      {0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f};
  constexpr const float quadTex[] =
      {0.0f, 0.0f,
       1.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f};
  constexpr const float quadTan[] =
      {1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f};
  constexpr const size_t quadIndices[] =
      {0, 1, 2,
       1, 3, 2};
  constexpr const uint32_t numberVertices = 4;
  constexpr const uint32_t numberIndices = 6;
  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});
  for (uint32_t i = 0; i < numberVertices; i++) {
    vertices[i] = {quadVertices[i * 4 + 0] * halfExtend + offset,
                   quadVertices[i * 4 + 1] * halfExtend + offset,
                   quadVertices[i * 4 + 2]};
    normals[i] = {quadNormal[i * 3 + 0],
                  quadNormal[i * 3 + 1],
                  quadNormal[i * 3 + 2]};
    texCoords[i] = {quadTex[i * 2 + 0],
                    quadTex[i * 2 + 1]};
    tangents[i] = {quadTan[i * 3 + 0],
                   quadTan[i * 3 + 1],
                   quadTan[i * 3 + 2],
                   1.0f};
  }
  std::vector<size_t> indices(quadIndices, quadIndices + numberIndices);
  return ImmutableModel(name,
                        std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableText::ImmutableText() noexcept = default;

ImmutableText::ImmutableText(const std::string& name, const std::filesystem::path& path) {
  auto size = std::filesystem::file_size(path);
  std::ifstream stream(path, std::ios_base::in | std::ios::binary);
  _text.resize(size, '\0');
  stream.read(_text.data(), size);
}

ImmutableText::~ImmutableText() noexcept = default;

const std::string& ImmutableText::GetName() const { return _name; }

bool ImmutableText::IsValid() const { return _text.empty(); }

void ImmutableText::Release() {
  _text.clear();
  _text.shrink_to_fit();
}

const std::string& ImmutableText::GetText() const { return _text; }

}  // namespace Hikari