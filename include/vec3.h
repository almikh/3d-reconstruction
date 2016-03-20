#ifndef VECTOR3_H_INCLUDED__
#define VECTOR3_H_INCLUDED__

#include <cmath>
#include <defs.h>
#include <vec2.h>

template<class T> class vec3 {
public:
  union {
    struct { T x, y, z; };
    T coords[3];
  };

  typedef T value_type;

  vec3() : x(0), y(0), z(0) {}
  vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  vec3(const vec2<T>& vert, ProjectionPlane plane, T other_coord = 0) :
    x(other_coord), y(other_coord), z(other_coord)
  {
    switch (plane) {
    case ProjectionPlane::OXY:
      x = vert.x;
      y = vert.y;
      break;
    case ProjectionPlane::OYZ:
      y = vert.x;
      z = vert.y;
      break;
    case ProjectionPlane::OXZ:
      x = vert.x;
      z = vert.y;
      break;
    }
  }

  vec3(const vec3<T>& vert) = default;

  template<class T2>
  vec3(const vec3<T2>& vert) : x(T(vert.x)), y(T(vert.y)), z(T(vert.z)) {}

  vec3<T>& operator=(const vec3<T>& rhs) {
    if (this == &rhs) return *this;
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
  }

  template<class T2>
  vec3& operator=(const vec3<T2>& rhs) {
    x = T(rhs.x);
    y = T(rhs.y);
    z = T(rhs.z);
    return *this;
  };

  vec3<T>& operator/=(const T& rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }
  vec3<T>& operator*=(const T& rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }
  vec3<T>& operator+=(const T& rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
    return *this;
  }
  vec3<T>& operator+=(const vec3<T>& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
  vec3<T>& operator-=(const T& rhs) {
    x -= rhs;
    y -= rhs;
    z -= rhs;
    return *this;
  }
  vec3<T>& operator-=(const vec3<T>& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  vec3<T>& normalize() {
    double t = sqrt(x*x + y*y + z*z);
    x /= t;
    y /= t;
    z /= t;
    return *this;
  }

  double length() const {
    return sqrt(x*x + y*y + z*z);
  }

  double dot(const vec3<T>& rhs) { /* скалярное произведение */
    return (x*rhs.x) + (y*rhs.y) + (z*rhs.z);
  }
  double angle(const vec3<T>& rhs) {
    return acos(double(x*rhs.x + y*rhs.y + z*rhs.z) / (length()*rhs.length()));
  }
  double dist(const vec3<T>& rhs) const {
    return sqrt(double((x - rhs.x)*(x - rhs.x) + (y - rhs.y)*(y - rhs.y) + (z - rhs.z)*(z - rhs.z)));
  }
  double sqrDist(const vec3<T>& rhs) const {
    return double((x - rhs.x)*(x - rhs.x) + (y - rhs.y)*(y - rhs.y) + (z - rhs.z)*(z - rhs.z));
  }

  inline vec2<T> projXY() const {
    return vec2<T>(x, y);
  }
  inline vec2<T> projXZ() const {
    return vec2<T>(x, z);
  }
  inline vec2<T> projYZ() const {
    return vec2<T>(y, z);
  }
  inline vec2<T> proj(ProjectionPlane plane) const {
    switch (plane) {
    case ProjectionPlane::OXY: return projXY();
    case ProjectionPlane::OYZ: return projYZ();
    case ProjectionPlane::OXZ: return projXZ();
    }
    return vec2<T>();
  }

  /* векторное произведение */
  vec3<T> cross(const vec3<T>& rhs) {
    vec3<T> result;
    result.x = y*rhs.z - z*rhs.y;
    result.y = z*rhs.x - x*rhs.z;
    result.z = x*rhs.y - y*rhs.x;
    return result;
  }

  template<class T2>
  vec3<T2> normalized() const {
    T2 t = std::sqrt(x*x + y*y + z*z);
    return vec3<T2>(x*1.0 / t, y*1.0 / t, z*1.0 / t);
  }

  template<class T2> vec3<T2> to() {
    return vec3<T2>(T2(x), T2(y), T2(z));
  }

  static vec3<T> i; /* Базисный вектор i */
  static vec3<T> j; /* Базисный вектор j */
  static vec3<T> k; /* Базисный вектор k */
};

typedef vec3<double> vec3d;
typedef vec3<float> vec3f;
typedef vec3<int> vec3i;
typedef vec3<uint8_t> vec3b;

template<class T> vec3<T> vec3<T>::i = vec3<T>(1, 0, 0);
template<class T> vec3<T> vec3<T>::j = vec3<T>(0, 1, 0);
template<class T> vec3<T> vec3<T>::k = vec3<T>(0, 0, 1);

#endif // VECTOR3_H_INCLUDED__
