#include <iostream>
#include <cstdlib>
#include <hikari/mathematics.h>

using namespace std;
using namespace Hikari;

using Vector3f = Vector<float, 3>;

template<typename Type, size_t Size>
bool AreEquals(const Vector<Type, Size> a, const Vector<Type, Size> b) {
  return a == b;
}

int main(int argc, char** argv) {
  Vector3f a(1, 3, 5);
  Vector3f b(-1, 3, 2);
  {
    auto&& c = a + b;
    cout << c << "\n";
    if (!AreEquals(c, Vector3f(0, 6, 7))) { return -1; }
  }
  {
    auto&& c = a - b;
    cout << c << "\n";
    if (!AreEquals(c, Vector3f(2, 0, 3))) { return -1; }
  }
  {
    auto&& c = a * b;
    cout << c << "\n";
    if (!AreEquals(c, Vector3f(-1, 9, 10))) { return -1; }
  }
  {
    auto&& c = a / b;
    cout << c << "\n";
    if (!AreEquals(c, Vector3f(-1, 1, 5.0f / 2.0f))) { return -1; }
  }
  {
    a += Vector3f(1);
    cout << a << "\n";
    if (!AreEquals(a, Vector3f(2, 4, 6))) { return -1; }
  }
  {
    a -= Vector3f(2);
    cout << a << "\n";
    if (!AreEquals(a, Vector3f(0, 2, 4))) { return -1; }
  }
  {
    a *= Vector3f(-1, 2, 3);
    cout << a << "\n";
    if (!AreEquals(a, Vector3f(0, 4, 12))) { return -1; }
  }
  {
    a /= Vector3f(1, 4, 2);
    cout << a << "\n";
    if (!AreEquals(a, Vector3f(0, 1, 6))) { return -1; }
  }
  {
    b = -b;
    cout << b << "\n";
    if (!AreEquals(b, Vector3f(1, -3, -2))) { return -1; }
  }

  {
    Vector3f x(0, 1, 1);
    Vector3f y(0, 1, 0);
    auto r = Dot(x, y);
    if (r != 1) { return -1; }
  }

  {
    Vector3f x(0, 1, 1);
    x = Normalize(x);
  }

  cout << "passed test" << "\n";
  return 0;
}