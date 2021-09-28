#pragma once

#include <iostream>
#include <limits>
#include <cmath>
#include <type_traits>
#include <cassert>

namespace Hikari {
constexpr auto PI_VALUE = 3.141592653589793;
constexpr auto PI = static_cast<float>(PI_VALUE);
constexpr auto EPSILON = std::numeric_limits<float>::epsilon();

/**
  * @brief 角度转弧度
  */
constexpr float Radian(float angle) { return PI / 180.0f * angle; }

/**
  * @brief 弧度转角度
  */
constexpr float Angle(float radian) { return 180.0f / PI * radian; }

template <typename Type, size_t Size>
struct Vector;
template <typename Type, size_t Row, size_t Column>
struct Matrix;
template <typename Type, std::enable_if_t<std::numeric_limits<Type>::is_iec559, int> = 0>
struct Quaternion;

/**
  * @brief 向量
  * @tparam Type 数据类型（比如float，double）
  */
template <typename Type, size_t Size>
struct Vector {
  Vector() noexcept = default;

  explicit Vector(Type t) noexcept {
    for (size_t i = 0; i < Size; i++) {
      v[i] = t;
    }
  }

  template <typename... Param, std::enable_if_t<sizeof...(Param) == Size, int> = 0>
  Vector(Param... params) noexcept {
    Type p[]{static_cast<Type>(params)...};
    for (size_t i = 0; i < Size; i++) {
      v[i] = p[i];
    }
  }

  Type& operator[](size_t i) {
    assert(i < Size);
    return v[i];
  }

  const Type& operator[](size_t i) const {
    assert(i < Size);
    return v[i];
  }

  Vector operator+(const Vector& vec) const {  //vec是加号右边,this是加号左边
    Vector result;
    for (size_t i = 0; i < Size; i++) {
      result[i] = (*this)[i] + vec[i];
    }
    return result;
  }

  Vector& operator+=(const Vector& vec) {
    for (size_t i = 0; i < Size; i++) {
      (*this)[i] += vec.v[i];
    }
    return *this;
  }

  Vector operator-(const Vector& vec) const {
    Vector result;
    for (size_t i = 0; i < Size; i++) {
      result[i] = (*this)[i] - vec[i];
    }
    return result;
  }

  Vector& operator-=(const Vector& vec) {
    for (size_t i = 0; i < Size; i++) {
      (*this)[i] -= vec.v[i];
    }
    return *this;
  }

  Vector operator-() const {
    Vector result;
    for (size_t i = 0; i < Size; i++) {
      result[i] = -(*this)[i];
    }
    return result;
  }

  Vector operator*(const Vector& vec) const {
    Vector result;
    for (size_t i = 0; i < Size; i++) {
      result[i] = (*this)[i] * vec[i];
    }
    return result;
  }

  Vector& operator*=(const Vector& vec) {
    for (size_t i = 0; i < Size; i++) {
      (*this)[i] *= vec.v[i];
    }
    return *this;
  }

  Vector operator/(const Vector& vec) const {
    Vector result;
    for (size_t i = 0; i < Size; i++) {
      result[i] = (*this)[i] / vec[i];
    }
    return result;
  }

  Vector& operator/=(const Vector& vec) {
    for (size_t i = 0; i < Size; i++) {
      (*this)[i] /= vec.v[i];
    }
    return *this;
  }

  bool operator==(const Vector& a) const {
    for (size_t i = 0; i < Size; i++) {
      if (v[i] != a.v[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Vector& a) const {
    return !operator==(a);
  }

  template <size_t S = Size, std::enable_if_t<(S >= 1), int> = 0>
  const Type& X() const { return v[0]; }
  template <size_t S = Size, std::enable_if_t<(S >= 1), int> = 0>
  Type& X() { return v[0]; }
  template <size_t S = Size, std::enable_if_t<(S >= 2), int> = 0>
  const Type& Y() const { return v[1]; }
  template <size_t S = Size, std::enable_if_t<(S >= 2), int> = 0>
  Type& Y() { return v[1]; }
  template <size_t S = Size, std::enable_if_t<(S >= 3), int> = 0>
  const Type& Z() const { return v[2]; }
  template <size_t S = Size, std::enable_if_t<(S >= 3), int> = 0>
  Type& Z() { return v[2]; }
  template <size_t S = Size, std::enable_if_t<(S >= 4), int> = 0>
  const Type& W() const { return v[3]; }
  template <size_t S = Size, std::enable_if_t<(S >= 4), int> = 0>
  Type& W() { return v[3]; }

  const Type* GetAddress() const { return v; }

  friend std::ostream& operator<<(std::ostream& out, const Vector& m) {
    out << '<';
    for (size_t i = 0; i < Size; i++) {
      out << m.v[i];
      if (i != Size - 1) {
        out << ',';
      }
    }
    out << '>';
    return out;
  }

 private:
  Type v[Size];
};

/**
  * @brief 点乘
  */
template <typename Value, size_t Size>
Value Dot(const Vector<Value, Size>& a1, const Vector<Value, Size>& a2) {
  Value result = static_cast<Value>(0);
  for (size_t i = 0; i < Size; i++) {
    result += a1[i] * a2[i];
  }
  return result;
}

/**
  * @brief 向量的模的平方
  */
template <typename Value, size_t Size>
Value SquaredLength(const Vector<Value, Size>& a) {
  return Dot(a, a);
}

/**
  * @brief 向量的模
  */
template <typename Value, size_t Size>
Value Length(const Vector<Value, Size>& a) {
  return std::sqrt(Dot(a, a));
}

/**
  * @brief 归一化
  */
template <typename Value, size_t Size>
Vector<Value, Size> Normalize(const Vector<Value, Size>& a) {
  return a / Vector<Value, Size>(Length(a));
}

/**
  * @brief 叉乘
  */
template <typename Value>
Vector<Value, 3> Cross(const Vector<Value, 3>& a, const Vector<Value, 3>& b) {
  return Vector<Value, 3>(
      a.Y() * b.Z() - a.Z() * b.Y(),
      a.Z() * b.X() - a.X() * b.Z(),
      a.X() * b.Y() - a.Y() * b.X());
}

template <typename Type, size_t Size>
Vector<Type, Size> Sin(const Vector<Type, Size>& vec) {
  Vector<Type, Size> result;
  for (size_t i = 0; i < Size; i++) {
    result[i] = std::sin(vec[i]);
  }
  return result;
}

template <typename Type, size_t Size>
Vector<Type, Size> Cos(const Vector<Type, Size>& vec) {
  Vector<Type, Size> result;
  for (size_t i = 0; i < Size; i++) {
    result[i] = std::cos(vec[i]);
  }
  return result;
}

template <typename Type, size_t Size>
Vector<Type, Size> Tan(const Vector<Type, Size>& vec) {
  Vector<Type, Size> result;
  for (size_t i = 0; i < Size; i++) {
    result[i] = std::tan(vec[i]);
  }
  return result;
}

template <typename Type, size_t Size>
Vector<Type, Size> Radian(const Vector<Type, Size>& vec) {
  Vector<Type, Size> result;
  for (size_t i = 0; i < Size; i++) {
    result[i] = Type(PI_VALUE) / Type(180) * vec[i];
  }
  return result;
}

template <typename Type, size_t Size>
Vector<Type, Size> Angle(const Vector<Type, Size>& vec) {
  Vector<Type, Size> result;
  for (size_t i = 0; i < Size; i++) {
    result[i] = Type(180) / Type(PI_VALUE) * vec[i];
  }
  return result;
}

/**
  * @brief 列优先矩阵
  * @tparam Type 数据类型（比如float，double）
  */
template <typename Type, size_t Row, size_t Column>
struct Matrix {
 public:
  template <size_t R = Row, size_t C = Column, std::enable_if_t<R == C, int> = 0>
  constexpr static Matrix<Type, Row, Column> Identity() {
    Matrix<Type, Row, Column> identify{};
    for (size_t i = 0; i < Row; i++) {
      identify.At(i, i) = static_cast<Type>(1);
    }
    return identify;
  }

  Matrix() noexcept = default;

  explicit Matrix(Type t) noexcept {
    for (size_t i = 0; i < Row; i++) {
      for (size_t j = 0; j < Column; j++) {
        m[i][j] = t;
      }
    }
  }

  template <size_t R, size_t C>
  explicit Matrix(const Matrix<Type, R, C>& mat) {
    for (size_t i = 0; i < Row; i++) {
      for (size_t j = 0; j < Column; j++) {
        if (i >= R || j >= C) {
          m[i][j] = 0;
        } else {
          m[i][j] = mat.At(i, j);
        }
      }
    }
  }

  template <typename... Param, std::enable_if_t<sizeof...(Param) == Row * Column, int> = 0>
  Matrix(Param... params) noexcept {
    Type p[]{static_cast<Type>(params)...};
    for (size_t i = 0; i < Row; i++) {
      for (size_t j = 0; j < Column; j++) {
        m[i][j] = p[i * Column + j];
      }
    }
  }

  Type& At(size_t i, size_t j) {
    assert(i < Row);
    assert(j < Column);
    return m[i][j];
  }

  const Type& At(size_t i, size_t j) const {
    assert(i < Row);
    assert(j < Column);
    return m[i][j];
  }

  const Type* GetAddress() const {
    return &m[0][0];
  }

  template <size_t RightRow, size_t RightColumn, std::enable_if_t<(Column == RightRow), int> = 0>
  Matrix<Type, Row, RightColumn> operator*(const Matrix<Type, RightRow, RightColumn>& mat) const {
    Matrix<Type, Row, RightColumn> result;
    for (size_t i = 0; i < Row; i++) {
      for (size_t j = 0; j < RightColumn; j++) {
        Type t = static_cast<Type>(0);
        for (size_t k = 0; k < Column; k++) {
          Type left = this->At(i, k);
          Type right = mat.At(k, j);
          t += left * right;
        }
        result.At(i, j) = t;
      }
    }
    return result;
  }

  template <size_t Size, std::enable_if_t<(Size == Column), int> = 0>
  Vector<Type, Row> operator*(const Vector<Type, Size>& vec) const {
    Vector<Type, Row> result;
    for (size_t i = 0; i < Row; i++) {
      Type t = static_cast<Type>(0);
      for (size_t j = 0; j < Size; j++) {
        Type left = this->At(i, j);
        Type right = vec[j];
        t += left * right;
      }
      result[i] = t;
    }
    return result;
  }

  /**
   * @brief 虽然是列优先矩阵，但是输出的时候按行优先输出（
  */
  friend std::ostream& operator<<(std::ostream& out, const Matrix& mat) {
    for (size_t i = 0; i < Row; i++) {
      out << '[';
      for (size_t j = 0; j < Column; j++) {
        out << mat.At(i, j);
        if (j != Column - 1) {
          out << ',';
        }
      }
      out << "]";
      if (i != Row - 1) {
        out << ',' << '\n';
      }
    }
    return out;
  }

 private:
  Type m[Row][Column];
};

/**
  * @brief 缩放矩阵
  */
template <typename Value>
Matrix<Value, 4, 4> Scale(const Vector<Value, 4>& v) {
  Matrix<Value, 4, 4> result{};
  result.At(0, 0) = v[0];
  result.At(1, 1) = v[1];
  result.At(2, 2) = v[2];
  result.At(3, 3) = v[3];
  return result;
}

/**
  * @brief 缩放矩阵
  */
template <typename Value>
Matrix<Value, 4, 4> Scale(const Vector<Value, 3>& v) {
  Matrix<Value, 4, 4> result{};
  result.At(0, 0) = v[0];
  result.At(1, 1) = v[1];
  result.At(2, 2) = v[2];
  result.At(3, 3) = static_cast<Value>(1);
  return result;
}

/**
  * @brief 位移矩阵
  */
template <typename Value>
Matrix<Value, 4, 4> Translate(const Vector<Value, 3>& v) {
  auto result = Matrix<Value, 4, 4>::Identity();
  result.At(3, 0) = v[0];
  result.At(3, 1) = v[1];
  result.At(3, 2) = v[2];
  return result;
}

/**
  * @brief 旋转矩阵
  * @param axis 旋转轴
  * @param radian 旋转弧度
  */
template <typename Value>
Matrix<Value, 4, 4> Rotate(const Vector<Value, 3>& axis, Value radian) {
  Value s = std::sin(radian);
  Value c = std::cos(radian);
  Value t = 1 - c;

  Matrix<Value, 4, 4> result(0);
  result.At(3, 3) = static_cast<Value>(1);
  result.At(0, 0) = c + axis.X() * axis.X() * t;
  result.At(1, 1) = c + axis.Y() * axis.Y() * t;
  result.At(2, 2) = c + axis.Z() * axis.Z() * t;

  Value tmp1 = axis.X() * axis.Y() * t;
  Value tmp2 = axis.Z() * s;
  result.At(0, 1) = tmp1 + tmp2;
  result.At(1, 0) = tmp1 - tmp2;

  tmp1 = axis.X() * axis.Z() * t;
  tmp2 = axis.Y() * s;
  result.At(0, 2) = tmp1 - tmp2;
  result.At(2, 0) = tmp1 + tmp2;

  tmp1 = axis.Y() * axis.Z() * t;
  tmp2 = axis.X() * s;
  result.At(1, 2) = tmp1 + tmp2;
  result.At(2, 1) = tmp1 - tmp2;

  return result;
}

/**
 * @note 从Unity.Mathematics抄来的
 * @brief 从欧拉角构造旋转矩阵，旋转顺序ZXY
*/
template <typename Type>
Matrix<Type, 4, 4> Rotate(const Vector<Type, 3>& eulerAngle) {
  Vector<Type, 3> c = Cos(Radian(eulerAngle));
  Vector<Type, 3> s = Sin(Radian(eulerAngle));

  Matrix<Type, 4, 4> result{};

  result.At(0, 0) = c.Y() * c.Z() + s.X() * s.Y() * s.Z();
  result.At(1, 0) = c.Z() * s.X() * s.Y() - c.Y() * s.Z();
  result.At(2, 0) = c.X() * s.Y();
  result.At(0, 1) = c.X() * s.Z();
  result.At(1, 1) = c.X() * c.Z();
  result.At(2, 1) = -s.X();
  result.At(0, 2) = c.Y() * s.X() * s.Z() - c.Z() * s.Y();
  result.At(1, 2) = c.Y() * c.Z() * s.X() + s.Y() * s.Z();
  result.At(2, 2) = c.X() * c.Y();
  result.At(3, 3) = Type(1);

  return result;
}

/**
  * @brief 透视投影矩阵
  * @param fov 视场（弧度）
  * @param aspect 纵横比
  * @param near 近平面
  * @param far 远平面
  */
template <typename Value>
Matrix<Value, 4, 4> Perspective(Value fov, Value aspect, Value near, Value far) {
  Value tanHalfFovy = std::tan(fov / static_cast<Value>(2));

  Matrix<Value, 4, 4> Result{};
  Result.At(0, 0) = static_cast<Value>(1) / (aspect * tanHalfFovy);
  Result.At(1, 1) = static_cast<Value>(1) / (tanHalfFovy);
  Result.At(2, 2) = -(far + near) / (far - near);
  Result.At(2, 3) = -static_cast<Value>(1);
  Result.At(3, 2) = -(static_cast<Value>(2) * far * near) / (far - near);
  return Result;
}

/**
  * @brief 正交投影矩阵
  */
template <typename Value>
Matrix<Value, 4, 4> Ortho(Value left, Value right, Value bottom, Value top, Value near, Value far) {
  Value rl = static_cast<Value>(1.0) / (right - left);
  Value tb = static_cast<Value>(1.0) / (top - bottom);
  Value fn = static_cast<Value>(1.0) / (far - near);

  Matrix<Value, 4, 4> result{};

  result.At(0, 0) = static_cast<Value>(2) * rl;
  result.At(1, 1) = static_cast<Value>(2) * tb;
  result.At(2, 2) = -static_cast<Value>(2) * fn;
  result.At(3, 3) = static_cast<Value>(1);
  result.At(3, 0) = -(right + left) * rl;
  result.At(3, 1) = -(top + bottom) * tb;
  result.At(3, 2) = -(far + near) * fn;

  return result;
}

/**
  * @brief 视图矩阵
  * @param origin 起始坐标
  * @param target 目标坐标
  * @param up 上方向向量
  */
template <typename Value>
Matrix<Value, 4, 4> LookAt(const Vector<Value, 3>& origin, const Vector<Value, 3>& target, const Vector<Value, 3>& up) {
  auto dir = Normalize(target - origin);
  auto right = Normalize(Cross(dir, up));
  auto newUp = Cross(right, dir);

  Matrix<Value, 4, 4> result{};
  result.At(0, 0) = right.X();
  result.At(1, 0) = right.Y();
  result.At(2, 0) = right.Z();
  result.At(0, 1) = newUp.X();
  result.At(1, 1) = newUp.Y();
  result.At(2, 1) = newUp.Z();
  result.At(0, 2) = -dir.X();
  result.At(1, 2) = -dir.Y();
  result.At(2, 2) = -dir.Z();
  result.At(3, 0) = -Dot(right, origin);
  result.At(3, 1) = -Dot(newUp, origin);
  result.At(3, 2) = Dot(dir, origin);
  result.At(3, 3) = static_cast<Value>(1.0);
  return result;
}

/**
 * @brief 矩阵转置
*/
template <typename Value, size_t Row, size_t Column>
Matrix<Value, Column, Row> Transpose(const Matrix<Value, Row, Column>& mat) {
  Matrix<Value, Column, Row> result;
  for (size_t i = 0; i < Row; i++) {
    for (size_t j = 0; j < Column; j++) {
      result.At(j, i) = mat.At(i, j);
    }
  }
  return result;
}

/**
 * @brief 矩阵求逆，使用高斯－若尔当（Gauss-Jordan）消元法
 * @param mat 原矩阵
 * @param result 逆矩阵
 * @return 原矩阵是否可以求逆矩阵
*/
template <typename Value, size_t N>
bool Invert(const Matrix<Value, N, N>& mat, Matrix<Value, N, N>& result) {
  static_assert(std::numeric_limits<Value>::is_iec559, "only IEC559(IEEE 754) Value type can Invert");
  Matrix<Value, N, N> m = mat;               //复制一份，运算过程中会将原矩阵变成单位矩阵
  result = Matrix<Value, N, N>::Identity();  //右边添加一个单位矩阵，扩展成增广矩阵
  for (size_t column = 0; column < N; column++) {
    //首先保证主对角线元素都不为0
    if (std::abs(m.At(column, column)) <= std::numeric_limits<Value>::epsilon()) {
      size_t big = column;
      //如果主对角线有0，则在该列中找出绝对值最大的那个，记录一下在哪一行
      for (size_t row = 0; row < N; row++) {
        if (std::abs(m.At(row, column)) > std::abs(m.At(big, column))) {
          big = row;
        }
      }
      if (big == column) {  //如果找不到，则不存在逆矩阵
        return false;
      } else {
        //交换行
        for (size_t j = 0; j < N; ++j) {
          std::swap(m.At(column, j), m.At(big, j));
          std::swap(result.At(column, j), result.At(big, j));
        }
      }
    }
    //将该列的每一行除主对角线元素都置0
    for (size_t row = 0; row < N; row++) {
      if (row != column) {
        auto coeff = -m.At(row, column) / m.At(column, column);
        if (coeff != 0) {
          for (size_t j = 0; j < N; j++) {
            m.At(row, j) += coeff * m.At(column, j);
            result.At(row, j) += coeff * result.At(column, j);
          }
          m.At(row, column) = 0;
        }
      }
    }
  }
  //最后将主对角线置1
  for (size_t row = 0; row < N; row++) {
    for (size_t column = 0; column < N; ++column) {
      result.At(row, column) /= m.At(row, row);
    }
  }
  return true;
}

/**
 * @note https://www.geometrictools.com/Documentation/ConstrainedQuaternions.pdf
 * @brief 旋转矩阵获取欧拉角，顺序ZXY
*/
template <typename Type>
Vector<Type, 3> EulerAngle(const Matrix<Type, 3, 3>& mat) {
  Vector<Type, 3> result{};
  if (mat.At(1, 2) < Type(1)) {
    if (mat.At(1, 2) > Type(-1)) {
      result.X() = std::asin(mat.At(1, 2));
      result.Z() = std::atan2(-mat.At(1, 0), mat.At(1, 1));
      result.Y() = std::atan2(-mat.At(0, 2), mat.At(2, 2));
    } else {
      result.X() = -Type(PI_VALUE) / Type(2);
      result.Z() = -std::atan2(-mat.At(2, 0), mat.At(0, 0));
      result.Y() = 0;
    }
  } else {
    result.X() = Type(PI_VALUE) / Type(2);
    result.Z() = std::atan2(-mat.At(2, 0), mat.At(0, 0));
    result.Y() = 0;
  }
  return Angle(result);
}

/**
 * @brief 旋转矩阵获取欧拉角，顺序ZXY
*/
template <typename Type>
Vector<Type, 3> EulerAngle(const Matrix<Type, 4, 4>& mat) {
  return EulerAngle(Matrix<Type, 3, 3>(mat));
}

/**
 * @brief 四元数
*/
template <typename Type, std::enable_if_t<std::numeric_limits<Type>::is_iec559, int>>
struct Quaternion : public Vector<Type, 4> {
  Quaternion() : Vector<Type, 4>() {}

  Quaternion(Type x, Type y, Type z, Type w) : Vector<Type, 4>(x, y, z, w) {}

  /**
   * @brief 旋转轴和旋转角转四元数
   * @param axis 旋转轴
   * @param angle 旋转角，单位：角度
  */
  Quaternion(const Vector<Type, 3>& axis, Type angle) {
    auto sinX = std::sin(Radian(angle * Type(0.5)));
    auto cosX = std::cos(Radian(angle * Type(0.5)));
    this->W() = cosX;
    this->X() = axis.X() * sinX;
    this->Y() = axis.Y() * sinX;
    this->Z() = axis.Z() * sinX;
  }

  /**
   * @note https://zhuanlan.zhihu.com/p/45404840
   * @brief 欧拉角转四元数，旋转顺序ZXY
   * @param eulerAngle 欧拉角，单位：角度
  */
  Quaternion(const Vector<Type, 3>& eulerAngle) {
    Vector<Type, 3> c = Cos(Radian(eulerAngle * Vector<Type, 3>(Type(0.5))));
    Vector<Type, 3> s = Sin(Radian(eulerAngle * Vector<Type, 3>(Type(0.5))));
    this->W() = c.X() * c.Y() * c.Z() + s.X() * s.Y() * s.Z();
    this->X() = s.X() * c.Y() * c.Z() + c.X() * s.Y() * s.Z();
    this->Y() = c.X() * s.Y() * c.Z() - s.X() * c.Y() * s.Z();
    this->Z() = c.X() * c.Y() * s.Z() - s.X() * s.Y() * c.Z();
  }

  /**
   * @note https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
   * @brief 从旋转矩阵构造四元数
   * @param mat 旋转矩阵
  */
  Quaternion(const Matrix<Type, 3, 3>& mat) {
    Vector<Type, 4> q{};
    Type t{};
    if (mat.At(2, 2) < 0) {
      if (mat.At(0, 0) > mat.At(1, 1)) {
        t = Type(1) + mat.At(0, 0) - mat.At(1, 1) - mat.At(2, 2);
        q = {t, mat.At(1, 0) + mat.At(0, 1), mat.At(0, 2) + mat.At(2, 0), mat.At(1, 2) - mat.At(2, 1)};
      } else {
        t = Type(1) - mat.At(0, 0) + mat.At(1, 1) - mat.At(2, 2);
        q = {mat.At(1, 0) + mat.At(0, 1), t, mat.At(2, 1) + mat.At(1, 2), mat.At(2, 0) - mat.At(0, 2)};
      }
    } else {
      if (mat.At(0, 0) < -mat.At(1, 1)) {
        t = Type(1) - mat.At(0, 0) - mat.At(1, 1) + mat.At(2, 2);
        q = {mat.At(0, 2) + mat.At(2, 0), mat.At(2, 1) + mat.At(1, 2), t, mat.At(0, 1) - mat.At(1, 0)};
      } else {
        t = Type(1) + mat.At(0, 0) + mat.At(1, 1) + mat.At(2, 2);
        q = {mat.At(1, 2) - mat.At(2, 1), mat.At(2, 0) - mat.At(0, 2), mat.At(0, 1) - mat.At(1, 0), t};
      }
    }
    q *= Vector<Type, 4>{Type(0.5) / std::sqrt(t)};
    this->X() = q.X();
    this->Y() = q.Y();
    this->Z() = q.Z();
    this->W() = q.W();
  }

  /**
   * @brief 从旋转矩阵构造四元数
   * @param mat 旋转矩阵
  */
  Quaternion(const Matrix<Type, 4, 4>& mat) : Quaternion(Matrix<Type, 3, 3>(mat)) {}
};

/**
 * @note https://zhuanlan.zhihu.com/p/45404840
 * @brief 使用四元数构造旋转矩阵
*/
template <typename Type>
Matrix<Type, 4, 4> Rotate(const Quaternion<Type>& q) {
  Matrix<Type, 4, 4> result{};

  Type xx = q.X() * q.X();
  Type yy = q.Y() * q.Y();
  Type zz = q.Z() * q.Z();

  Type xy = q.X() * q.Y();
  Type xz = q.X() * q.Z();
  Type xw = q.X() * q.W();

  Type yz = q.Y() * q.Z();
  Type yw = q.Y() * q.W();

  Type zw = q.Z() * q.W();

  result.At(3, 3) = Type(1);
  result.At(0, 0) = 1 - 2 * yy - 2 * zz;
  result.At(1, 1) = 1 - 2 * xx - 2 * zz;
  result.At(2, 2) = 1 - 2 * xx - 2 * yy;

  result.At(1, 0) = 2 * xy - 2 * zw;
  result.At(0, 1) = 2 * xy + 2 * zw;

  result.At(2, 0) = 2 * xz + 2 * yw;
  result.At(0, 2) = 2 * xz - 2 * yw;

  result.At(2, 1) = 2 * yz - 2 * xw;
  result.At(1, 2) = 2 * yz + 2 * xw;

  return result;
}

using Vector2f = Vector<float, 2>;
using Vector3f = Vector<float, 3>;
using Vector4f = Vector<float, 4>;
using Vector2i = Vector<int, 2>;
using Vector3i = Vector<int, 3>;
using Vector4i = Vector<int, 4>;
using Matrix2f = Matrix<float, 2, 2>;
using Matrix3f = Matrix<float, 3, 3>;
using Matrix4f = Matrix<float, 4, 4>;
using Quaternionf = Quaternion<float>;

}  // namespace Hikari