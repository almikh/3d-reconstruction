#ifndef DEFS_H_INCLUDED__
#define DEFS_H_INCLUDED__
#include <limits>

using Int = std::numeric_limits<double>;
using Double = std::numeric_limits<double>;

template<class T>
class Type : public std::numeric_limits < T > {};

namespace math {
	static const double Pi = 3.14159265358979323846;
	static const double Pi_2 = 1.57079632679489661923;
	static const double Pi_4 = 0.78539816339744830962;

	template<class T> 
	inline T sqr(T value) {
		return value*value;
	}

	template<class T>
	inline int sign(const T& val) {
		static_assert(std::is_integral<T>::value, "Integer required.");

		if (val == 0) return 0;
		return (val < 0) ? -1 : 1;
	}
}

enum class ProjectionPlane {
	OXY,
	OYZ,
	OXZ
};

enum class LinesRatio {
	NotIntersect,
	Parallel,
	Coinside,
	Intersect
};

#endif // DEFS_H_INCLUDED__
