#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <stdexcept>

#include <hikari/mathematics.h>

namespace Hikari {
class AssetLoadException : public std::runtime_error {
 public:
  explicit AssetLoadException(const std::string& msg) noexcept : std::runtime_error(msg.c_str()) {}
  explicit AssetLoadException(const char* msg) noexcept : std::runtime_error(msg) {}
};

/**
  * @brief 资源基类
  */
class Asset : public std::enable_shared_from_this<Asset> {
 public:
  Asset() noexcept;
  Asset(const Asset&) = delete;
  virtual ~Asset() noexcept = 0;
  /**
     * @brief 资源索引名字
    */
  virtual const std::string& GetName() const = 0;
  /**
     * @brief 资源实例是否可用
    */
  virtual bool IsValid() const = 0;
  /**
     * @brief 释放资源
    */
  virtual void Release() = 0;
};

/**
  * @brief 8bit 位图
  */
class ImmutableBitmap : public Asset {
 public:
  ImmutableBitmap() noexcept;
  /**
    * @brief 从硬盘加载
    * @param name 资源索引名字
    * @param path 路径
    * @param isFilpY 是否颠倒Y轴
    */
  ImmutableBitmap(const std::string& name, const std::filesystem::path& path, bool isFilpY);
  ImmutableBitmap(const ImmutableBitmap&) = delete;
  ImmutableBitmap(ImmutableBitmap&&) noexcept;
  ImmutableBitmap& operator=(ImmutableBitmap&&) noexcept;
  ~ImmutableBitmap() noexcept override;
  const std::string& GetName() const override;
  bool IsValid() const override;
  void Release() override;

  int GetWidth() const;
  int GetHeight() const;
  /**
    * @brief 图像通道数量
    */
  int GetChannel() const;
  const uint8_t* const GetData() const;

  static bool LoadFromDisk(const std::string&, const std::filesystem::path&, bool filpY, ImmutableBitmap&);

 private:
  std::string _name;
  int _width{};
  int _height{};
  int _channelCount{};
  uint8_t* _data{nullptr};
};

/**
 * @brief 32bit HDR图片
*/
class ImmutableHdrTexture : public Asset {
 public:
  ImmutableHdrTexture() noexcept;
  ImmutableHdrTexture(const std::string& name, const std::filesystem::path& path, bool isFilpY);
  ImmutableHdrTexture(const ImmutableHdrTexture&) = delete;
  ImmutableHdrTexture(ImmutableHdrTexture&&) noexcept;
  ImmutableHdrTexture& operator=(ImmutableHdrTexture&&) noexcept;
  ~ImmutableHdrTexture() noexcept override;
  const std::string& GetName() const override;
  bool IsValid() const override;
  void Release() override;

  int GetWidth() const;
  int GetHeight() const;
  int GetChannel() const;
  const float* const GetData() const;

 private:
  std::string _name;
  int _width{};
  int _height{};
  int _channelCount{};
  float* _data{nullptr};
};

/**
  * @brief 模型
  */
class ImmutableModel : public Asset {
 public:
  ImmutableModel() noexcept;
  /**
    * @brief 从硬盘加载
    * @param name 资源索引名字
    * @param path 路径
    */
  ImmutableModel(const std::string& name, const std::filesystem::path& path);
  ImmutableModel(const std::string& name,
                 std::vector<Vector3f>&& pos,
                 std::vector<Vector3f>&& nor,
                 std::vector<Vector2f>&& tex,
                 std::vector<size_t>&& ind) noexcept;
  ImmutableModel(const ImmutableModel&) = delete;
  ImmutableModel(ImmutableModel&&) noexcept;
  ImmutableModel& operator=(ImmutableModel&&) noexcept;
  ~ImmutableModel() noexcept override;
  const std::string& GetName() const override;
  bool IsValid() const override;
  void Release() override;

  const std::vector<Vector3f>& GetPosition() const;
  const std::vector<Vector3f>& GetNormals() const;
  const std::vector<Vector2f>& GetTexCoords() const;
  const std::vector<size_t>& GetIndices() const;
  /**
    * @brief 是否存在法线
    */
  bool HasNormal() const;
  /**
    * @brief 是否存在纹理坐标
    */
  bool HasTexCoord() const;
  size_t GetVertexCount() const;
  size_t GetIndexCount() const;

  static bool LoadFromFile(const std::string&, const std::filesystem::path&, ImmutableModel&);
  static ImmutableModel CreateSphere(const std::string& name, float radius, int numberSlices);
  static ImmutableModel CreateCube(const std::string& name, float halfExtend);

 private:
  std::vector<Vector3f> _positions;
  std::vector<Vector3f> _normals;
  std::vector<Vector2f> _texcoords;
  std::vector<size_t> _indices;
  std::string _name;
};

class ImmutableText : public Asset {
 public:
  ImmutableText() noexcept;
  ImmutableText(const std::string& name, const std::filesystem::path& path);
  ~ImmutableText() noexcept override;
  const std::string& GetName() const override;
  bool IsValid() const override;
  void Release() override;

  const std::string& GetText() const;

 private:
  std::string _text;
  std::string _name;
};

}  // namespace Hikari