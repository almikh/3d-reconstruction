#ifndef MISC_H_INCLUDED__
#define MISC_H_INCLUDED__
#include <utility>
#include <triangle.h>

template<class T>
std::pair<Triangle<T>, Triangle<T>> trianglesFromRect(const std::vector<int>& rectIndices) {
  std::pair<Triangle<T>, Triangle<T>> dst;
  dst.first.indices[0] = rectIndices[0];
  dst.second.indices[0] = dst.first.indices[1] = rectIndices[1];
  dst.second.indices[1] = dst.first.indices[2] = rectIndices[2];
  dst.second.indices[2] = rectIndices[3];

  return dst;
}

#endif // MISC_H_INCLUDED__
