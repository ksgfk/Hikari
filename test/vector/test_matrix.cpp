#include <iostream>
#include <vector>
#include <hikari/mathematics.h>

using namespace std;
using namespace Hikari;

int main() {
  Matrix<float, 1, 4> m(1, 2, 3, 4);
  cout << m << "\n\n";

  Matrix<float, 4, 1> n(1, 2, 3, 4);
  cout << n << "\n\n";

  Matrix<float, 3, 2> a(1, 2, 3, 4, 5, 6);
  cout << a << "\n\n";

  Matrix<float, 2, 3> b(1, 2, 3, 4, 5, 6);
  cout << b << "\n\n";

  cout << Matrix<float, 3, 3>::Identity() << "\n\n";

  auto c = Matrix<float, 4, 4>::Identity();
  c.At(0, 3) = 1;
  c.At(1, 3) = 3;
  c.At(2, 3) = 2;
  cout << c << "\n\n";
  auto d = Matrix<float, 4, 1>(2, 6, 4, 1);
  auto e = c * d;
  cout << e << "\n\n";

  auto h = Matrix<float, 4, 4>::Identity();
  h.At(3, 0) = 1;
  h.At(3, 1) = 3;
  h.At(3, 2) = 2;
  cout << c << "\n\n";
  auto f = Matrix<float, 1, 4>(2, 6, 4, 1);
  auto g = f * h;
  cout << g << "\n\n";

  Matrix4f mvp = Matrix4f::Identity() * Scale<float>({1, 2, 1});
  Vector4f homo = {1, 2, 3, 1};
  auto r = mvp * homo;
  cout << r << endl;

  //Matrix<float, 4, 4> r = Rotate<float>(Vector<float, 3>(0, 1, 0), 90);
  Matrix<float, 4, 4> t = Perspective<float>(3.1415926f / 180 * 45, 1280.0f / 720, 0.01f, 100.0f);
  //Matrix<float, 4, 4> o = Ortho<float>(-1, 1, -1, 1, 0.01f, 100.0f);
  //Matrix<float, 4, 4> l = LookAt<float>(Vector<float, 3>(0, 1, 0), Vector<float, 3>(0, -1, 0), Vector<float, 3>(0, 0, 1));

  cout << t << "\n";
  cout << LookAt<float>({0, 0, 0}, {0, 1, 0}, {0, 0, 1}) << "\n";

  cout << "-----------------Transpose-----------------\n";

  Matrix4f mat(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  auto tr = Transpose(mat);
  cout << mat << "\n";
  cout << tr << "\n";

  cout << "-----------------Inv-----------------\n";

  Matrix3f ti(2, -1, 0, -1, 2, -1, 0, -1, 2);
  Matrix3f invTi;
  Invert(ti, invTi);
  cout << ti << "\n";
  cout << invTi << "\n";

  cout << "\n";

  Matrix4f ia(1, 2, 4, 1, 2, 0, 2, 3, 3, 3, 0, 1, 4, 3, 5, 0);
  Matrix4f invIa;
  Invert(ia, invIa);
  cout << ia << "\n";
  cout << invIa << "\n"
       << endl;
  cout << "\n";

  Matrix3f id(1, 2, 5, 2, 5, 1, 8, 6, 2);
  Matrix3f invid;
  Invert(id, invid);
  cout << id << "\n";
  cout << invid << "\n";

  cout << "\n";

  Matrix3f ifa(3, 7, 2, 2, 2, 4, 4, 9, 3);
  Matrix3f invifa;
  bool canInv = Invert(ifa, invifa);
  cout << canInv;

  cout << endl;

  return 0;
}