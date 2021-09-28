#include <hikari/camera.h>

#include <algorithm>

namespace Hikari {
Camera::~Camera() = default;

Matrix4f Camera::GetSkyboxViewMatrix() const {
  return Matrix4f(Matrix3f(GetViewMatrix()));  //去掉位移
}

PerspectiveCamera::PerspectiveCamera() noexcept = default;

PerspectiveCamera::~PerspectiveCamera() = default;

Vector3f PerspectiveCamera::GetPosition() const {
  return Position;
}

Vector3f PerspectiveCamera::GetLookTarget() const {
  return Target;
}

Vector3f PerspectiveCamera::GetUpDirection() const {
  return Up;
}

void PerspectiveCamera::SetPosition(const Vector3f& pos) {
  Position = pos;
}

void PerspectiveCamera::SetLookTarget(const Vector3f& target) {
  Target = target;
}

void PerspectiveCamera::SetUpDirection(const Vector3f& up) {
  Up = up;
}

void PerspectiveCamera::SetAspect(int width, int height) {
  Aspect = static_cast<float>(width) / static_cast<float>(height);
}

Matrix4f PerspectiveCamera::GetViewMatrix() const {
  return LookAt<float>(Position, Target, Up);
}

Matrix4f PerspectiveCamera::GetProjectionMatrix() const {
  return Perspective<float>(Radian(Fov), Aspect, ZNear, ZFar);
}

OrthoCamera::OrthoCamera() noexcept = default;

OrthoCamera::~OrthoCamera() = default;

Matrix4f OrthoCamera::GetViewMatrix() const {
  return LookAt<float>(Position, Target, Up);
}

Matrix4f OrthoCamera::GetProjectionMatrix() const {
  return Ortho<float>(Left, Right, Bottom, Top, ZNear, ZFar);
}

Vector3f OrthoCamera::GetPosition() const {
  return Position;
}

Vector3f OrthoCamera::GetLookTarget() const {
  return Target;
}

Vector3f OrthoCamera::GetUpDirection() const {
  return Up;
}

void OrthoCamera::SetPosition(const Vector3f& pos) {
  Position = pos;
}

void OrthoCamera::SetLookTarget(const Vector3f& target) {
  Target = target;
}

void OrthoCamera::SetUpDirection(const Vector3f& up) {
  Up = up;
}

void OrthoCamera::SetAspect(int width, int height) {
  float a = static_cast<float>(width) / static_cast<float>(height);
  Left = -Size * a;
  Right = Size * a;
  Top = Size;
  Bottom = -Size;
}

OrbitControls::OrbitControls() noexcept = default;

void OrbitControls::Update(int height, Vector2f cursorPos, Vector2f dolly, bool isRotate, bool isPan) {
  auto cursorDelta = cursorPos - _cursePos;
  UpdateWithDelta(height, cursorDelta, dolly, isRotate, isPan);
  _cursePos = cursorPos;
}

void OrbitControls::UpdateWithDelta(int height, Vector2f cursorDelta, Vector2f dolly, bool isRotate, bool isPan) {
  if (isRotate) {
    _deltaOrbit = cursorDelta / Vector2f((float)height);
  } else {
    _deltaOrbit = Vector2f(0);
  }
  if (isPan) {
    _deltaPan = cursorDelta * Vector2f(PanSpeed);
  } else {
    _deltaPan = Vector2f(0);
  }
  _dolly = dolly;
}

void OrbitControls::ApplyCamera(Camera* camera) const {
  const auto camPos = camera->GetPosition();
  const auto camTar = camera->GetLookTarget();
  const auto camUp = camera->GetUpDirection();
  if (_deltaOrbit != Vector2f(0) || _dolly.Y() != 0) {
    auto&& fromTarget = camPos - camTar;
    float radius = Length(fromTarget);
    float theta = std::atan2(fromTarget.X(), fromTarget.Z());
    float phi = std::acos(fromTarget.Y() / radius);
    float factor = PI * 2 * OrbitSpeed;
    radius *= std::pow(0.95f, _dolly.Y());
    auto orbit = _deltaOrbit;
    auto t = theta - orbit.X() * factor;
    auto p = phi - orbit.Y() * factor;
    p = std::clamp(p, EPSILON, PI - EPSILON);
    Vector3f offset(0);
    offset.X() = radius * (float)sin(p) * (float)sin(t);
    offset.Y() = radius * (float)cos(p);
    offset.Z() = radius * (float)sin(p) * (float)cos(t);
    camera->SetPosition(camTar + offset);
  }
  if (_deltaPan != Vector2f(0)) {
    auto&& forward = camTar - camPos;
    auto&& forwardDir = Normalize(forward);
    auto&& forwardDis = Length(forward);
    auto&& left = Cross(camUp, forwardDir);
    auto&& up = Cross(forwardDir, left);
    float factor = forwardDis * std::tan(45.0f / 2.0f) * 2.0f;
    auto&& x = left * Vector3f(_deltaPan.X() * factor);
    auto&& y = up * Vector3f(_deltaPan.Y() * factor);
    auto&& delta = x + y;
    camera->SetLookTarget(camTar + delta);
    camera->SetPosition(camPos + delta);
  }
}
}  // namespace Hikari