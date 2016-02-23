#ifndef AABB_H_INCLUDED__
#define AABB_H_INCLUDED__
#include <defs.h>
#include <vec3.h>

template<class T>
struct AABB {
  vec3<T> min, max;

  AABB() = default;
  AABB(T _min, T _max) : min(_max, _max, _max), max(_min, _min, _min) {

  }

  vec3<T> center() const {
    return vec3i((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2);
  }

  bool intersect(const AABB<T>& aabb) {
    if (max.x < aabb.min.x || min.x > aabb.max.x) return false;
    if (max.y < aabb.min.y || min.y > aabb.max.y) return false;
    if (max.z < aabb.min.z || min.z > aabb.max.z) return false;

    return true;
  }

  inline bool contains(T x, T y, T z) {
    return x >= min.x && y >= min.y && z >= min.z && x <= max.x && y <= max.y && z <= max.z;
  }
  inline bool contains(const vec3<T>& p) {
    return contains(p.x, p.y, p.z);
  }

  inline bool inside(T x, T y, T z) {
    return x>min.x && y>min.y && z>min.z && x<max.x && y<max.y && z<max.z;
  }
  inline bool inside(const vec3<T>& p) {
    return contains(p.x, p.y, p.z);
  }
};

template<class T, class SrcIt>
AABB<T> createAABB(SrcIt begin, SrcIt end) {
  AABB<T> dst(-(std::numeric_limits<T>::max() - 1), std::numeric_limits<T>::max());
  for (; begin != end; ++begin) {
    dst.min.x = std::min(dst.min.x, begin->x);
    dst.min.y = std::min(dst.min.y, begin->y);
    dst.min.z = std::min(dst.min.z, begin->z);
    dst.max.x = std::max(dst.max.x, begin->x);
    dst.max.y = std::max(dst.max.y, begin->y);
    dst.max.z = std::max(dst.max.z, begin->z);
  }

  return dst;
}

#endif // AABB_H_INCLUDED__
