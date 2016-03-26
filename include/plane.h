#ifndef PLANE_H_INCLUDED__
#define PLANE_H_INCLUDED__

#include <vec3.h>
#include <matrix.h>
#include <algebra.h>

template<class T>
class Plane {
public:
  T A, B, C, D;

  Plane() : A(0), B(0), C(0), D(0) {}
  Plane(T _A, T _B, T _C, T _D) : A(_A), B(_B), C(_C), D(_D) {}
  template<class T2>
  Plane(const vec3<T2>& f, const vec3<T2>& s, const vec3<T2>& t) {
    create(f, s, t);
  }

  template<class T2>
  Plane<T>& create(const vec3<T2>& f, const vec3<T2>& s, const vec3<T2>& t) {
		mat3d matA, matB, matC, matD;
		matA(0, 0) = 1.0; matA(0, 1) = 1.0; matA(0, 2) = 1.0;
		matA(1, 0) = f.y; matA(1, 1) = s.y; matA(1, 2) = t.y;
		matA(2, 0) = f.z; matA(2, 1) = s.z; matA(2, 2) = t.z;

		matB(0, 0) = f.x; matB(0, 1) = s.x; matB(0, 2) = t.x;
		matB(1, 0) = 1.0; matB(1, 1) = 1.0; matB(1, 2) = 1.0;
		matB(2, 0) = f.z; matB(2, 1) = s.z; matB(2, 2) = t.z;

		matC(0, 0) = f.x; matC(0, 1) = s.x; matC(0, 2) = t.x;
		matC(1, 0) = f.y; matC(1, 1) = s.y; matC(1, 2) = t.y;
		matC(2, 0) = 1.0; matC(2, 1) = 1.0; matC(2, 2) = 1.0;

		matD(0, 0) = f.x; matD(0, 1) = s.x; matD(0, 2) = t.x;
		matD(1, 0) = f.y; matD(1, 1) = s.y; matD(1, 2) = t.y;
		matD(2, 0) = f.z; matD(2, 1) = s.z; matD(2, 2) = t.z;

    A = static_cast<T>(matA.det());
		B = static_cast<T>(matB.det());
		C = static_cast<T>(matC.det());
		D = static_cast<T>(matD.det());

    return *this;
  }

  /* расстояние от точки М до этой плоскости */
  double dist(const vec3<T>& M) const {
    return abs(A*M.x + B*M.y + *M.z + D) / sqrt(double(A*A + B*B + C*C));
  }

  /* угол между плоскостями */
  double angle(const Plane<T>& plane) const {
    return normal().angle(plane.angle());
  }

  /* угол между лучом и плоскостью */
  double angle(const vec3<T>& straight) const {
    return normal().angle(straight);
  }

  vec3<T> normal() const {
    return vec3<T>(A, B, C);
  }

	// пересечение плоскости и луча, заданного началом и направлением
	bool intersect(const vec3<T>& origin, const vec3<T>& dir) {
		vec3<T> n = normal();
		double alpha = n.dot(dir);

		if (std::abs(alpha) > Double::epsilon()) {
			double result = (n.dot(origin) - D) / alpha;
			return (result >= 0.0);
		}

		return false;
	}
};

#endif // PLANE_H_INCLUDED__
