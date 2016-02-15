#ifndef ALGEBRA_H_INCLUDED__
#define ALGEBRA_H_INCLUDED__

#include <defs.h>
#include <matrix.h>
#include <vec3.h>

namespace slae {
	/* Метод алгебраической прогонки */
	void diag3rd(double* a, double* b, double* c, double* d, double* dst, int n);
}

template <class T> 
vec2<T> operator+(const vec2<T>& lhs, const vec2<T>& rhs) {
	return vec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}

template <class T> 
vec2<T> operator-(const vec2<T>& lhs, const vec2<T>& rhs) {
	return vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}

template <class T>
bool operator==(const vec2<T>& lhs, const vec2<T>& rhs) {
	return (lhs.x == rhs.x && lhs.y == rhs.y);
}

template <class T> 
vec2<T> operator*(const vec2<T>& lhs, const vec2<T>& rhs) {
	return vec2<T>(lhs.x * rhs.x, lhs.y * rhs.y);
}

template <class T> 
vec2<T> operator/(const vec2<T>& lhs, const vec2<T>& rhs) {
	return vec2<T>(lhs.x / rhs.x, lhs.y / rhs.y);
}

template <class T>
vec2<T> operator/(const vec2<T>& lhs, const T& rhs) {
	return vec2<T>(lhs.x / rhs, lhs.y / rhs);
}

template <class T>
vec2<T> operator*(const T& lhs, const vec2<T>&& rhs) {
	return vec2<T>(rhs.x*lhs, rhs.y*lhs);
}

template <class T>
vec2<T> operator*(const vec2<T>& lhs, const T& rhs) {
	return vec2<T>(lhs.x*rhs, lhs.y*rhs);
}

template <class T> 
vec3<T> operator/(const vec3<T>& lhs, const T& rhs) {
  return vec3<T>(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

template <class T> 
vec3<T> operator*(const T& lhs, const vec3<T>&& rhs) {
  return vec3<T>(rhs.x*lhs, rhs.y*lhs, rhs.z*lhs);
}

template <class T> 
vec3<T> operator*(const vec3<T>& lhs, const T& rhs) {
  return vec3<T>(lhs.x*rhs, lhs.y*rhs, lhs.z*rhs);
}

template <class T> 
vec3<T> operator+(const vec3<T>& lhs, const vec3<T>& rhs) {
  return vec3<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template <class T> 
vec3<T> operator-(const vec3<T>& lhs, const vec3<T>& rhs) {
  return vec3<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template <class T> bool operator==(const vec3<T>& lhs, const vec3<T>& rhs) {
  return lhs.x==rhs.x && lhs.y==rhs.y && lhs.z==rhs.z;
}

template <class T> 
bool operator!=(const vec3<T>& lhs, const vec3<T>& rhs) {
  return lhs.x!=rhs.x || lhs.y!=rhs.y || lhs.z!=rhs.z;
}

template<class T, int dim>
matrix<T, dim> operator +(const matrix<T, dim>& lhs, const matrix<T, dim>& rhs) {
  matrix<T, dim> src;
  T* cur = src.data();
  T* curL = lhs.data();
  T* curR = rhs.data();
  for(int i = 0; i<dim*dim; ++i) *cur++ = (*curL++) + (*curR++);
  return src;
}

template<class T, int dim>
matrix<T, dim> operator -(const matrix<T, dim>& lhs, const matrix<T, dim>& rhs) {
  matrix<T, dim> src;
  T* cur = src.data();
  T* curL = lhs.data();
  T* curR = rhs.data();
  for(int i = 0; i<dim*dim; ++i) *cur++ = (*curL++) - (*curR++);
  return src;
}

template<class T, int dim>
matrix<T, dim> operator *(const matrix<T, dim>& lhs, const matrix<T, dim>& rhs) {
  matrix<T, dim> src;
  for(int i = 0; i<dim; ++i) {
    for(int j = 0; j<dim; ++j) {
      T temp = 0;
      for(int r = 0; r<dim; ++r) temp += lhs(i, r)*rhs(r, j);
      src(i, j) = temp;
    }
  }
  return src;
}

template<class T>
vec3<T> operator *(const vec3<T>& rhs, const matrix<T, 3>& lhs) {
  return vec3<T>(
        lhs(0, 0)*rhs.x + lhs(1, 0)*rhs.y + lhs(2, 0)*rhs.z,
        lhs(0, 1)*rhs.x + lhs(1, 1)*rhs.y + lhs(2, 1)*rhs.z,
        lhs(0, 2)*rhs.x + lhs(1, 2)*rhs.y + lhs(2, 2)*rhs.z
        );
}

template<class T>
vec3<T> operator *(const matrix<T, 3>& lhs, const vec3<T>& rhs) {
	return vec3<T>(
		lhs(0, 0)*rhs.x + lhs(0, 1)*rhs.y + lhs(0, 2)*rhs.z,
		lhs(1, 0)*rhs.x + lhs(1, 1)*rhs.y + lhs(1, 2)*rhs.z,
		lhs(2, 0)*rhs.x + lhs(2, 1)*rhs.y + lhs(2, 2)*rhs.z
		);
}

#endif // ALGEBRA_H_INCLUDED__
