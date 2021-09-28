#include <iostream>

#include <hikari/mathematics.h>

using namespace std;
using namespace Hikari;

int main() {
  Quaternionf q(Vector3f{28.710f, 9.097f, 2.333f});
  cout << q << endl;

  Quaternionf a(Normalize(Vector3f{1, -1, 0}), 32);
  cout << a << endl;

  Matrix4f am = Rotate(a);
  Matrix4f bm = Rotate<float>(Normalize(Vector3f{1, -1, 0}), Radian(32));
  cout << am << endl;
  cout << bm << endl;

  auto ea = EulerAngle(bm);
  cout << ea << endl;

  Quaternionf mtq(bm);
  cout << mtq << endl;

  Matrix4f eam = Rotate(Vector3f{20, 35, -70});
  cout << eam << endl;
  Matrix4f duizhao = Rotate(Quaternionf(Vector3f{20, 35, -70}));
  cout << duizhao << endl;

  Quaternionf z(1, 2, 3, 4);
  cout << z << endl;

  Quaternionf zc = z;
  cout << zc << endl;

  return 0;
}