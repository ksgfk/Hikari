#pragma once

#include <hikari/mathematics.h>

namespace Hikari {
class Camera {
 public:
  virtual ~Camera() = 0;
  virtual Matrix4f GetViewMatrix() const = 0;
  virtual Matrix4f GetProjectionMatrix() const = 0;
  virtual Vector3f GetPosition() const = 0;
  virtual Vector3f GetLookTarget() const = 0;
  virtual Vector3f GetUpDirection() const = 0;
  virtual void SetPosition(const Vector3f& pos) = 0;
  virtual void SetLookTarget(const Vector3f& target) = 0;
  virtual void SetUpDirection(const Vector3f& up) = 0;
  virtual void SetAspect(int width, int height) = 0;

  Matrix4f GetSkyboxViewMatrix() const;
};

class PerspectiveCamera : public Camera {
 public:
  PerspectiveCamera() noexcept;
  ~PerspectiveCamera() override;
  Matrix4f GetViewMatrix() const override;
  Matrix4f GetProjectionMatrix() const override;
  Vector3f GetPosition() const override;
  Vector3f GetLookTarget() const override;
  Vector3f GetUpDirection() const override;
  void SetPosition(const Vector3f& pos) override;
  void SetLookTarget(const Vector3f& target) override;
  void SetUpDirection(const Vector3f& up) override;
  void SetAspect(int width, int height) override;

 public:
  float Fov = 45;
  float Aspect{1.77777f};
  float ZNear{0.01f};
  float ZFar{100.0f};
  Vector3f Position{0, 0, -5};
  Vector3f Target{0, 0, 0};
  Vector3f Up{0, 1, 0};
};

class OrthoCamera : public Camera {
 public:
  OrthoCamera() noexcept;
  ~OrthoCamera() override;
  Matrix4f GetViewMatrix() const override;
  Matrix4f GetProjectionMatrix() const override;
  Vector3f GetPosition() const override;
  Vector3f GetLookTarget() const override;
  Vector3f GetUpDirection() const override;
  void SetPosition(const Vector3f& pos) override;
  void SetLookTarget(const Vector3f& target) override;
  void SetUpDirection(const Vector3f& up) override;
  void SetAspect(int width, int height) override;

 public:
  float Top = 1;
  float Bottom = -1;
  float Left = -1;
  float Right = 1;
  float ZNear = 0.01f;
  float ZFar = 100.0f;
  float Size = 1.0f;
  Vector3f Position{0, 0, -5};
  Vector3f Target{0, 0, 0};
  Vector3f Up{0, 1, 0};
};

class OrbitControls {
 private:
  Vector2f _cursePos{};
  Vector2f _deltaOrbit{};
  Vector2f _deltaPan{};
  Vector2f _dolly{};

 public:
  OrbitControls() noexcept;
  void Update(int height, Vector2f cursorPos, Vector2f dolly, bool isRotate, bool isPan);
  void UpdateWithDelta(int height, Vector2f cursorDelta, Vector2f dolly, bool isRotate, bool isPan);
  void ApplyCamera(Camera* camera) const;

  float PanSpeed = 0.0008f;
  float OrbitSpeed = 0.25f;
};
}  // namespace Hikari