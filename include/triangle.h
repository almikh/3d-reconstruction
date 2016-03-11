#ifndef TRIANGLE_H_INCLUDED__
#define TRIANGLE_H_INCLUDED__

#include <array>
#include <vec3.h>

template<class T>
class Triangle {
  std::array<int, 3> indices_;

public:
  vec3<T> normal;

  Triangle(int first = 0, int second = 0, int third = 0) :
    normal(0, 0, 0)
  {
    indices_[0] = first;
    indices_[1] = second;
    indices_[2] = third;
  }

  Triangle(const Triangle<T>& tri) {
    indices_ = tri.indices_;
    normal = tri.normal;
  }

  Triangle(Triangle<T>&& tri) {
    indices_ = std::move(tri.indices_);
    normal = tri.normal;
  }

  Triangle<T>& operator=(const Triangle<T>& tri) {
    if (this == &tri) return *this;
    indices_ = tri.indices_;
    normal = tri.normal;

    return *this;
  }

  Triangle<T>& operator=(Triangle<T>&& tri) {
    indices_ = std::move(tri.indices_);
    normal = tri.normal;

    return *this;
  }

  int& operator[](int ind) {
    return indices_[ind];
  }

  int operator[](int ind) const {
    return indices_[ind];
  }
};

typedef Triangle<double> Trid;
typedef Triangle<int> Trii;

#endif // TRIANGLE_H_INCLUDED__
