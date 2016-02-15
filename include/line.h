#ifndef LINE_H_INCLUDED__
#define LINE_H_INCLUDED__

#include <defs.h>
#include <vec2.h>
#include <vec3.h>

/* ������ */
template<class T>
class Line {
	Line<T>& create(const vec2<T>& f, const vec2<T>& s) {
		A = f.y - s.y;
		B = s.x - f.x;
		C = f.x*s.y - s.x*f.y;
		return *this;
	}

public:
	T A, B, C;

	Line() : A(0), B(0), C(0) {}
	Line(T _A, T _B, T _C) : A(_A), B(_B), C(_C) {}
	Line(const vec3<T>& f, const vec3<T>& s, ProjectionPlane toPlane) {
		create(f.proj(toPlane), s.proj(toPlane));
	}

	/* �� ��, �� �� ���� - �������� ��� ���������� vec3'�� */
	template<class Iter> 
	Line(Iter begin, ProjectionPlane toPlane) {
		create((*begin).proj(toPlane), (*(begin + 1)).proj(toPlane));
	}

	Line(const vec2<T>& f, const vec2<T>& s) {
		create(f, s);
	}

	/* ����. 'a' ��� ��������� x/a + y/b = 1 */
	template<class T2>
	inline T2 getA() const {
		return static_cast<T2>(double(-C) / A);
	}
	/* ����. 'b' ��� ��������� x/a + y/b = 1 */
	template<class T2>
	inline T2 getB() const {
		return static_cast<T2>(double(-C) / B);
	}

	/* ������� ����. 'k' ��� ��������� y = kx+b */
	template<class T2>
	inline T2 getK() const {
		return static_cast<T2>(double(-A) / B);
	}

	bool parallel(const Line<T>& line) {
		return A*line.B - line.A*B == 0;
	}
	bool ortho(const Line<T>& line) {
		return A*line.A + line.B*B == 0;
	}

	/* ����, �� ������� ���� ���������  line ������ ����� ����������� ������ ������� ������� �� ������� ���������� � ������ */
	double angle(const Line<T>& line) const {
		return atan2(double(A*line.B - line.A*B), double(A*line.A + line.B*B));
	}
	/* ���������� �� ����� 'm' �� ������ */
	double dist(const vec2<T>& m) const {
		return abs(double(m.x*A + m.y*B + C)) / sqrt(A*A + B*B);
	}
	/* ����� ����������� ������ */
	vec2<T> intersect(const Line<T>& line) const {
		double denom = static_cast<double>(A*line.B - line.A*B);
		return vec2<T>(static_cast<T>(double(B*line.C - line.B*C) / denom), static_cast<T>(double(C*line.A - line.C*A) / denom));
	}
	/* ������� � ������ */
	vec2<T> normal() const {
		return vec2<T>(A, B);
	}
	/* ������������ ������ � ������ */
	vec2<T> direction() const {
		return vec2<T>(B, -A);
	}
};

#endif // LINE_H_INCLUDED__
